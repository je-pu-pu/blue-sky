#include "pch.h"

#include <core/sound/PortAudio/SoundEngine.h>
#include <core/sound/PortAudio/SoundBuffer.h>
#include <core/sound/OggVorbisFile.h>

#include <filesystem>
#include <memory>
#include <vector>

// ---- 音の速さ : 出力と周波数の違う音楽（22050 Hz）が、正しい長さで鳴ること ----
// 耳で聞かなくても確かめられるよう、本物のエンジン・本物の音楽ファイル・本物の読み出し（read_frame）で、鳴り終わるまでの出力のフレーム数を数える

namespace
{

std::string music_path( const char* name )
{
	return ( std::filesystem::path( __FILE__ ).parent_path() / "../../media/music" / name ).lexically_normal().string();
}

} // namespace

TEST( PlaybackRateTest, 22050Hzの音楽は出力の周波数で鳴らすと元の長さで鳴り終わる )
{
	// エンジンは作業ディレクトリ（source/）からの相対の場所で音色（SoundFont）を読むので、テストの間だけ移る
	struct CurrentPathScope
	{
		std::filesystem::path previous = std::filesystem::current_path();
		CurrentPathScope() { std::filesystem::current_path( std::filesystem::path( __FILE__ ).parent_path() / "../.." ); }
		~CurrentPathScope() { std::filesystem::current_path( previous ); }
	} current_path_scope;

	std::unique_ptr< core::sound::port_audio::SoundEngine > engine;

	try
	{
		engine = std::make_unique< core::sound::port_audio::SoundEngine >();
	}
	catch ( const std::exception& e )
	{
		std::cout << "音の出力の機器を開けないので確かめない : " << e.what() << std::endl;	// gtest 1.8 には GTEST_SKIP が無い
		return;
	}

	for ( const char* name : { "road.ogg", "semi.ogg", "washer.ogg" } )
	{
		OggVorbisFile file( music_path( name ).c_str() );
		const auto& format = file.format();
		ASSERT_EQ( 22050, format.sampling_rate ) << name;

		// 先頭の 2 秒ぶんだけ読む（全部だと時間がかかる）
		const size_t frame_bytes = sizeof( s16_t ) * format.channels;
		const size_t source_frames = 22050 * 2;
		std::vector< u8_t > data( source_frames * frame_bytes );
		file.read( data.data(), data.size() );

		// エンジンの出力に登録しない（登録すると出力のスレッドも同時に読み進めるため）。周波数の合わせ方はエンジンの出力の形式で決まる
		auto buffer = std::make_unique< core::sound::port_audio::SoundBuffer >( * engine, format, data.size() );

		std::memcpy( buffer->lock( 0, data.size() ), data.data(), data.size() );
		buffer->unlock();
		buffer->play( false );

		const int output_rate = engine->get_format().sampling_rate;
		size_t output_frames = 0;
		float l = 0.f, r = 0.f;

		while ( buffer->is_playing() && output_frames < static_cast< size_t >( output_rate ) * 10 )
		{
			buffer->read_frame( l, r );
			output_frames++;
		}

		// 2 秒の音は、出力の周波数でちょうど 2 秒（output_rate * 2 フレーム）鳴る。
		// 以前は周波数を合わせず、22050 Hz の音を 44100 Hz で鳴らすと半分（1 秒）で鳴り終わっていた（倍の速さ）
		const double seconds = static_cast< double >( output_frames ) / output_rate;
		EXPECT_NEAR( 2.0, seconds, 0.01 ) << name << " : 出力の周波数 " << output_rate;
	}
}
