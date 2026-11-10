#include "pch.h"

#include <blue_sky/stage/LegacyStageParser.h>
#include <blue_sky/stage/TomlStageParser.h>

#include <filesystem>
#include <fstream>

/**
 * 同梱の 16 個のステージで「古い読み方（.stage）の結果 == TOML の読み方（.stage.toml）の結果」を確かめる
 *
 * .stage.toml は tools/stage_convert/convert_stage.py で .stage から作った。変換で捨てた物（どこも読まない行・余った数・
 * 見つからない名前を指す動き）は、古い読み方でも読んだ結果に入らない（警告にする）ので、読んだ結果は同じになる。
 * TOML の側は警告 0 で読めること（変換で捨てた物の警告は、変換の報告と .stage.toml のコメントに残る）
 */

using blue_sky::stage::LegacyStageParser;
using blue_sky::stage::TomlStageParser;
using blue_sky::stage::StageData;

namespace
{

std::filesystem::path media_path( const string_t& relative )
{
	return ( std::filesystem::path( __FILE__ ).parent_path() / "../../media" / relative ).lexically_normal();
}

const char* const stages[] =
{
	"stage/0-0", "stage/0-1", "stage/0-2", "stage/0-3",
	"stage/1-0", "stage/1-1", "stage/1-2", "stage/1-3",
	"stage/2-0", "stage/2-1", "stage/2-2", "stage/2-3",
	"stage_ext/ext-0", "stage_ext/ext-1", "stage_ext/ext-2", "stage_ext/staff",
};

} // namespace

TEST( StageFormatCompareTest, 同梱の16個で古い読み方とTOMLの読み方の結果が同じ )
{
	for ( const char* stage : stages )
	{
		SCOPED_TRACE( stage );

		std::ifstream in( media_path( string_t( stage ) + ".stage" ) );
		ASSERT_TRUE( in.good() );

		const StageData legacy = LegacyStageParser::parse( in, string_t( stage ) + ".stage" );
		const StageData toml = TomlStageParser::parse_file( media_path( string_t( stage ) + ".stage.toml" ).string() );

		EXPECT_EQ( describe( legacy ), describe( toml ) );

		string_t warnings;

		for ( const auto& w : toml.warnings )
		{
			warnings += w + "\n";
		}

		EXPECT_TRUE( toml.warnings.empty() ) << warnings;
	}
}
