#pragma once
#include "utils.hh"

class riot_player {
	nlohmann::json data;
public:
	explicit riot_player( nlohmann::json a ) : data( std::move( a ) ) {}

	SETUP_VARIABLE( std::string, subject( ), "Subject" )
	SETUP_VARIABLE( std::string, character_id( ), "CharacterID" )
	SETUP_VARIABLE( std::string, player_card_id( ), "PlayerIdentity", "PlayerCardID" )
	SETUP_VARIABLE( bool, incognito( ), "PlayerIdentity", "Incognito" )

	[[nodiscard]] std::string get_name( const std::string& pd_url, const cpr::Header& header ) const {
		const auto body = nlohmann::json::array( { subject( ) } );

		const auto req = Put(
			cpr::Url{ std::string( pd_url ).append( R"(/name-service/v2/players)" ) },
			header,
			cpr::Body{ body.dump( ) }
		);

		if ( req.error.code != cpr::ErrorCode::OK || !nlohmann::json::accept( req.text ) )
			return "";

		const auto parsed_json = nlohmann::json::parse( req.text ).at( 0 );
		if ( !parsed_json.contains( "GameName" ) )
			return "";

		auto game_name = parsed_json.at( "GameName" ).get<std::string>( );
		const auto tag_line = parsed_json.at( "TagLine" ).get<std::string>( );

		return game_name.append( "#" ).append( tag_line );
	}

	[[nodiscard]] std::string get_agent( ) const {
		if ( !agents.contains( character_id( ) ) )
			return std::string( "Unknown Agent: " ).append( character_id( ) );

		return agents.at( character_id( ) );
	}
};