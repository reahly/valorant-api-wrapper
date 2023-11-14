# valorant-api-wrapper
Still WIP, gonna work on adding more APIs

Example Usage:
```c
#include "riot_client.hh"

int main() {
	auto client = riot_client( "eu" );

	if ( !client.init( ) )
		return -1;

	const auto user_info = client.user_info( );
	if ( user_info.has_value( ) ) {
		std::cout << "game_name: " << user_info->game_name << "#" << user_info->tag_line << std::endl;
		std::cout << "country: " << user_info->country << std::endl;
		std::cout << "username: " << user_info->username << std::endl;
		std::cout << "email verified: " << user_info->email_verified << std::endl;
		std::cout << "phone number verified: " << user_info->phone_number_verified << std::endl;
	}

	auto match_info = client.current_match_info( );
	if ( match_info.has_value( ) ) {
		std::cout << "State: " << match_info->state( ) << std::endl;
		std::cout << "MapID: " << match_info->map_id( ) << std::endl;
		std::cout << "ModeID: " << match_info->mode_id( ) << std::endl;
		std::cout << "Server Host: " << match_info->game_server_host( ) << ":" << match_info->game_server_port( ) << std::endl;
		std::cout << "Queue ID: " << match_info->queue_id( ) << std::endl;
		std::cout << "Is Ranked: " << match_info->is_ranked( ) << std::endl;

		for ( const auto& player : match_info->get_players( ) ) {
			const auto name = player.get_name( client.pd_url, client.online_header );

			std::cout << name << " Playing as " << player.get_agent( ) << ( player.incognito( ) ? " ( hidden name )" : "" ) << std::endl;
		}
	}

	system( "pause" );
}

```
