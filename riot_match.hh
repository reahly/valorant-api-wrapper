#pragma once
#include <string>
#include "riot_player.hh"

class riot_match {
	nlohmann::json data;
public:
	explicit riot_match( nlohmann::json a ) : data( std::move( a ) ) {}

	std::vector<riot_player> get_players( ) {
		std::vector<riot_player> ret;
		for ( const auto& player : data.at( "Players" ).items( ) ) {
			ret.emplace_back(
				player.value( )
			);
		}

		return ret;
	}

	SETUP_VARIABLE( std::string, state( ), "State" )
	SETUP_VARIABLE( std::string, map_id( ), "MapID" )
	SETUP_VARIABLE( std::string, mode_id( ), "ModeID" )
	SETUP_VARIABLE( std::string, game_server_host( ), "ConnectionDetails", "GameServerHost" )
	SETUP_VARIABLE( int, game_server_port( ), "ConnectionDetails", "GameServerPort" )
	SETUP_VARIABLE( std::string, queue_id( ), "MatchmakingData", "QueueID" )
	SETUP_VARIABLE( bool, is_ranked( ), "MatchmakingData", "IsRanked" )
};
