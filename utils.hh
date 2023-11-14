#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

template <typename T>
T get_nested_value( const nlohmann::json& container, const std::string& key ) {
	try {
		return container.at( key ).get<T>( );
	} catch ( ... ) {
		return T{ };
	}
}

template <typename T, typename... Keys>
T get_nested_value( const nlohmann::json& container, const std::string& key, const Keys&... keys ) {
	try {
		return get_nested_value<T>( container.at( key ), keys... );
	} catch ( ... ) {
		return T{ };
	}
}

#define SETUP_VARIABLE(type, function_name, ...) \
    [[nodiscard]] type function_name const { \
	    return get_nested_value<type>(data, __VA_ARGS__); \
    }

inline std::unordered_map<std::string, std::string> agents {
	{ "add6443a-41bd-e414-f6ad-e58d267f4e95", "Jett" },
	{ "a3bfb853-43b2-7238-a4f1-ad90e9e46bcc", "Reyna" },
	{ "f94c3b30-42be-e959-889c-5aa313dba261", "Raze" },
	{ "7f94d92c-4234-0a36-9646-3a87eb8b5c89", "Yoru" },
	{ "eb93336a-449b-9c1b-0a54-a891f7921d69", "Phoenix" },
	{ "bb2a4828-46eb-8cd1-e765-15848195d751", "Neon" },
	{ "5f8d3a7f-467b-97f3-062c-13acf203c006", "Breach" },
	{ "6f2a04ca-43e0-be17-7f36-b3908627744d", "Skye" },
	{ "320b2a48-4d9b-a075-30f1-1f93a9b638fa", "Sova" },
	{ "601dbbe7-43ce-be57-2a40-4abd24953621", "Kayo" },
	{ "1e58de9c-4950-5125-93e9-a0aee9f98746", "Killjoy" },
	{ "117ed9e3-49f3-6512-3ccf-0cada7e3823b", "Cypher" },
	{ "569fdd95-4d10-43ab-ca70-79becc718b46", "Sage" },
	{ "22697a3d-45bf-8dd7-4fec-84a9e28c69d7", "Chamber" },
	{ "8e253930-4c05-31dd-1b6c-968525494517", "Omen" },
	{ "9f0d8ba9-4140-b941-57d3-a7ad57c6b417", "Brimstone" },
	{ "41fb69c1-4189-7b37-f117-bcaf1e96f1bf", "Astra" },
	{ "707eab51-4836-f488-046a-cda6bf494859", "Viper" },
	{ "dade69b4-4f5a-8528-247b-219e5a1facd6", "Fade" },
	{ "95b78ed7-4637-86d9-7e41-71ba8c293152", "Harbor" },
	{ "e370fa57-4757-3604-3648-499e1f642d3f", "Gekko" },
	{ "cc8b64c8-4b25-4ff9-6e7f-37b4da43d235", "Deadlock" },
	{ "0e38b510-41a8-5780-5e8f-568b2a4f2d6c", "Iso" }
};

namespace utils {
	inline std::string read_file( const std::string& path ) {
		if ( !std::filesystem::exists( path ) )
			return { };

		std::ifstream file( path.data( ), std::ios::binary );
		if ( !file.good( ) )
			return { };

		file.unsetf( std::ios::skipws );

		file.seekg( 0, std::ios::end );
		const size_t size = file.tellg( );
		file.seekg( 0, std::ios::beg );

		std::vector<char> out;

		out.resize( size );
		file.read( out.data( ), size );
		file.close( );

		return std::string( out.begin( ), out.end( ) );
	}

	inline std::vector<std::string> split_string( const std::string& s, const char delim ) {
		std::stringstream ss( s );
		std::string item;
		std::vector<std::string> elems;
		while ( std::getline( ss, item, delim ) ) {
			elems.push_back( std::move( item ) );
		}

		return elems;
	}

	inline std::string base64_encode( const std::string& decoded_string ) {
		const auto base64_chars = std::string( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" );

		std::string out;

		int val = 0, valb = -6;
		for ( const unsigned char c : decoded_string ) {
			val = ( val << 8 ) + c;
			valb += 8;
			while ( valb >= 0 ) {
				out.push_back( base64_chars[val >> valb & 0x3F] );
				valb -= 6;
			}
		}
		if ( valb > -6 )
			out.push_back( base64_chars[val << 8 >> valb + 8 & 0x3F] );
		while ( out.size( ) % 4 )
			out.push_back( '=' );

		return out;
	}

	inline std::string url_encode( const std::string& input ) {
		std::string output;
		output.reserve( input.size( ) );

		for ( const char c : input ) {
			if ( std::isalnum( c ) || c == '-' || c == '_' || c == '.' || c == '~' ) {
				output += c;
			} else {
				output += '%';
				output += "0123456789ABCDEF"[c >> 4];
				output += "0123456789ABCDEF"[c & 15];
			}
		}

		return output;
	}

}