#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "utils.hh"
#include <cpr/cpr.h>
#include "riot_match.hh"

struct token_request_t {
	std::string access_token, issuer, subject, token;
};

struct user_info_t {
	std::string game_name, tag_line, country, username;
	bool email_verified, phone_number_verified;
};

struct friends_info_t {
	std::string activate_platform, display_group, game_name, game_tag, group, note, pid, puuid, region;
	uintptr_t last_online;
};

struct friend_request_info_t {
	std::string game_name, game_tag, note, pid, puuid, region;
	bool received;
};

class riot_client {
	std::string current_version, platform_type, region, local_authorization, online_authorization;
	token_request_t token_request;

public:
	cpr::Header online_header;
	std::string local_url, glz_url, pd_url;

	explicit riot_client( const std::string& region_ ) {
		region = region_;

		nlohmann::json platform;
		platform["platformType"] = "PC";
		platform["platformOS"] = "Windows";
		platform["platformOSVersion"] = "";
		platform["platformChipset"] = "Unknown";

		platform_type = utils::base64_encode( platform.dump( ) );
		current_version = get_current_valorant_version( );

		const auto lockfile_path = std::format( R"({}\Riot Games\Riot Client\Config\lockfile)", getenv( "LOCALAPPDATA" ) );
		if ( !std::filesystem::exists( lockfile_path ) )
			return;

		glz_url = std::format( R"(https://glz-{}-1.{}.a.pvp.net)", region, region /*fix*/ );
		pd_url = std::format( R"(https://pd.{}.a.pvp.net)", region );

		if ( const auto file = utils::read_file( lockfile_path ); !file.empty( ) ) {
			const auto splitted_file = utils::split_string( file, ':' );
			if ( splitted_file.size( ) < 5 )
				return;

			local_url = std::format( R"(https://127.0.0.1:{})", splitted_file.at( 2 ) );
			local_authorization = std::string( "Basic " ).append( utils::base64_encode( std::format( "riot:{}", splitted_file.at( 3 ) ) ) );
		} else {
			//TODO: login by user & pw
		}
	}

	static std::string get_current_valorant_version( ) {
		const auto req = Get(
			cpr::Url{ "https://valorant-api.com/v1/version" },
			cpr::VerifySsl{ false }
		);

		if ( req.error.code != cpr::ErrorCode::OK || !nlohmann::json::accept( req.text ) )
			return "";

		const auto parsed_json = nlohmann::json::parse( req.text );
		return std::format( "{}-shipping-{}-{}", parsed_json.at( "data" ).at( "branch" ).get<std::string>( ), parsed_json.at( "data" ).at( "buildVersion" ).get<std::string>( ), utils::split_string( parsed_json.at( "data" ).at( "version" ).get<std::string>( ), '.' ).at( 3 ) );
	}

	bool init( ) {
		const auto req = Get(
			cpr::Url{ std::string( local_url ).append( R"(/entitlements/v1/token)" ) },
			cpr::Header{ { "Authorization", local_authorization } },
			cpr::VerifySsl{ false }
		);

		if ( req.error.code != cpr::ErrorCode::OK || !nlohmann::json::accept( req.text ) )
			return false;

		const auto parsed_json = nlohmann::json::parse( req.text );
		if ( !parsed_json.contains( "accessToken" ) )
			return false;

		token_request = token_request_t{ parsed_json.at( "accessToken" ).get<std::string>( ), parsed_json.at( "issuer" ).get<std::string>( ), parsed_json.at( "subject" ).get<std::string>( ), parsed_json.at( "token" ).get<std::string>( ), };
		online_authorization = std::string( "Bearer " ).append( token_request.access_token );
		online_header = cpr::Header{
			{ "Authorization", online_authorization },
			{ "X-Riot-Entitlements-JWT", token_request.token },
			{ "X-Riot-ClientPlatform", platform_type },
			{ "X-Riot-ClientVersion", current_version },
		};

		return true;
	}

	std::optional<user_info_t> user_info( ) {
		const auto req = Get(
			cpr::Url{ std::string( local_url ).append( R"(/riot-client-auth/v1/userinfo)" ) },
			cpr::Header{ { "Authorization", local_authorization } },
			cpr::VerifySsl{ false }
		);

		if ( req.error.code != cpr::ErrorCode::OK || !nlohmann::json::accept( req.text ) )
			return std::nullopt;

		const auto parsed_json = nlohmann::json::parse( req.text );
		if ( !parsed_json.contains( "acct" ) )
			return std::nullopt;

		return user_info_t{
			parsed_json.at( "acct" ).at( "game_name" ).get<std::string>( ),
			parsed_json.at( "acct" ).at( "tag_line" ).get<std::string>( ),
			parsed_json.at( "country" ).get<std::string>( ),
			parsed_json.at( "preferred_username" ).get<std::string>( ),
			parsed_json.at( "email_verified" ).get<bool>( ),
			parsed_json.at( "phone_number_verified" ).get<bool>( )
		};
	}

	std::optional<riot_match> current_match_info( ) {
		const auto req = Get(
			cpr::Url{ std::string( glz_url ).append( std::format( R"(/core-game/v1/players/{})", token_request.subject ) ) },
			online_header,
			cpr::VerifySsl{ false }
		);

		if ( req.error.code != cpr::ErrorCode::OK )
			return std::nullopt;

		if ( !nlohmann::json::accept( req.text ) )
			return std::nullopt;

		const auto parsed_json = nlohmann::json::parse( req.text );
		if ( !parsed_json.contains( "MatchID" ) )
			return std::nullopt;

		const auto match_id = parsed_json.at( "MatchID" ).get<std::string>( );

		const auto match_req = Get(
			cpr::Url{ std::string( glz_url ).append( std::format( R"(/core-game/v1/matches/{})", match_id ) ) },
			online_header,
			cpr::VerifySsl{ false }
		);

		if ( match_req.error.code != cpr::ErrorCode::OK || !nlohmann::json::accept( match_req.text ) )
			return std::nullopt;

		return riot_match( nlohmann::json::parse( match_req.text ) );
	}

	std::vector<friends_info_t> friends( ) {
		const auto req = Get(
			cpr::Url{ std::string( local_url ).append( R"(/chat/v4/friends)" ) },
			cpr::Header{ { "Authorization", local_authorization } },
			cpr::VerifySsl{ false }
		);

		if ( req.error.code != cpr::ErrorCode::OK )
			return {};

		if ( !nlohmann::json::accept( req.text ) )
			return {};

		const auto parsed_json = nlohmann::json::parse( req.text );
		if ( !parsed_json.contains( "friends" ) )
			return {};

		std::vector<friends_info_t> ret;
		for ( const auto& i : parsed_json.at( "friends" ).items( ) ) {
			ret.emplace_back( friends_info_t{
				i.value( ).at( "activePlatform" ).is_null( ) ? "" : i.value( ).at( "activePlatform" ).get<std::string>( ),
				i.value( ).at( "displayGroup" ).get<std::string>( ),
				i.value( ).at( "game_name" ).get<std::string>( ),
				i.value( ).at( "game_tag" ).get<std::string>( ),
				i.value( ).at( "group" ).get<std::string>( ),
				i.value( ).at( "note" ).get<std::string>( ),
				i.value( ).at( "pid" ).get<std::string>( ),
				i.value( ).at( "puuid" ).get<std::string>( ),
				i.value( ).at( "region" ).get<std::string>( ),
				i.value( ).at( "last_online_ts" ).is_null( ) ? 0 : i.value( ).at( "last_online_ts" ).get<uintptr_t>( ),
			} );
		}

		return ret;
	}

	std::vector<friend_request_info_t> friend_requests( ) {
		const auto req = Get(
			cpr::Url{ std::string( local_url ).append( R"(/chat/v4/friendrequests)" ) },
			cpr::Header{ { "Authorization", local_authorization } },
			cpr::VerifySsl{ false }
		);

		if ( req.error.code != cpr::ErrorCode::OK )
			return {};

		if ( !nlohmann::json::accept( req.text ) )
			return {};

		const auto parsed_json = nlohmann::json::parse( req.text );
		if ( !parsed_json.contains( "requests" ) )
			return {};

		std::vector<friend_request_info_t> ret;
		for ( const auto& i : parsed_json.at( "requests" ).items( ) ) {
			ret.emplace_back( friend_request_info_t{
				i.value( ).at( "game_name" ).get<std::string>( ),
				i.value( ).at( "game_tag" ).get<std::string>( ),
				i.value( ).at( "note" ).get<std::string>( ),
				i.value( ).at( "pid" ).get<std::string>( ),
				i.value( ).at( "puuid" ).get<std::string>( ),
				i.value( ).at( "region" ).get<std::string>( ),
				i.value( ).at( "subscription" ).get<std::string>( ).find( "pending_in" ) != std::string::npos,
			} );
		}

		return ret;
	}
};
