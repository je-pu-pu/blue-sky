#include "SoundEngine.h"
#include <core/sound/SoundBuffer.h>
#include <core/sound/SoundFilter.h>
#include <core/sound/Sound.h>
#include <core/sound/MidiSynthesizer.h>
#include <core/type.h>

#include <game/SoundFormat.h>
#include <common/exception.h>

#include <portaudio/pa_win_wasapi.h>

#include <thread>
#include <chrono>

#include <algorithm>
#include <ranges>
#include <cmath>

namespace core::sound::port_audio
{

SoundEngine::SoundEngine()
{
	PaError e = Pa_Initialize();

	if ( e != paNoError )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( string_t( "Pa_Initialize() failed.\n" ) + Pa_GetErrorText( e ) );
	}

	// WASAPI のホストAPI から既定出力デバイスを取得する。
	// システム既定デバイス(Pa_GetDefaultOutputDevice)は MME/DirectSound 等
	// 別ホストAPI のことがあり、その場合 WASAPI 用ストリーム情報と不整合になる。
	const auto wasapi_api = Pa_HostApiTypeIdToHostApiIndex( paWASAPI );
	if ( wasapi_api < 0 )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "WASAPI host API is not available." );
	}
	const auto output_device = Pa_GetHostApiInfo( wasapi_api )->defaultOutputDevice;

	auto info = Pa_GetDeviceInfo( output_device );

	const auto channel_count = 2;
	const auto sample_format = paFloat32;
	const auto sample_rate = 44100; // info->defaultSampleRate; // 44100;

	format_ = {
		.channels = channel_count,
		.sampling_rate = static_cast< int >( sample_rate ),
		.bit_depth = 32
	};

	// midi_synthesizer_.reset( new MidiSynthesizer( "media/music/SGM-V2.01.sf2", format_ ) );
	// 音色は再配布できる Fluid R3 GM（MIT）。git には入れず tools/soundfont/fetch_soundfont.py で入手する（doc/ライセンス.md）
	midi_synthesizer_.reset( new MidiSynthesizer( "media/music/FluidR3_GM.sf2", format_ ) );

	auto wasapi_streawm_info = PaWasapiStreamInfo{
		.size = sizeof( PaWasapiStreamInfo ),
		.hostApiType = paWASAPI,
		.version = 1,
		.flags = paWinWasapiAutoConvert,
	};

	const auto output_params = PaStreamParameters{
		.device = output_device,
		.channelCount = channel_count,
		.sampleFormat = sample_format,
		.suggestedLatency = info->defaultLowOutputLatency,
		.hostApiSpecificStreamInfo = & wasapi_streawm_info,
	};

	e = Pa_OpenStream( & stream_, nullptr, & output_params, sample_rate, paFramesPerBufferUnspecified, paNoFlag, & callback, this );
	// e = Pa_OpenDefaultStream( & stream_, 0, 2, sample_format, sample_rate, paFramesPerBufferUnspecified, SoundEngine::callback, this );

	if ( e != paNoError )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( string_t( "Pa_OpenDefaultStream() failed.\n" ) + Pa_GetErrorText( e ) );
	}

	e = Pa_StartStream( stream_ );

	if ( e != paNoError )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( string_t( "Pa_StartStream() failed.\n" ) + Pa_GetErrorText( e ) );
	}
}

SoundEngine::~SoundEngine()
{
	Pa_StopStream( stream_ );

	Pa_CloseStream( stream_ );

	Pa_Terminate();
}

int SoundEngine::callback( const void*, void* output, unsigned long frame_count, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* user_data )
{
	auto sound_engine = static_cast< SoundEngine* >( user_data );
	auto out = static_cast< float* >( output );

	sound_engine->midi_synthesizer_->render( out, frame_count  );
	// tsf_render_float( sound_font, out, frame_count );

	std::lock_guard< std::mutex > lock( sound_engine->callback_mutex_ );

	// 3D 音源は「このコールバック内で位置一定」とみなし、先頭で各バッファの L/R ゲインを一度だけ算出。
	// 非3D音（BGM/UI 等）と MIDI は従来どおり位置なし（ゲイン 1）。
	for ( auto sb: sound_engine->sound_buffer_list_ )
	{
		if ( ! sb->is_playing() )
		{
			continue;
		}
		if ( sb->is_3d() )
		{
			float gl, gr;
			compute_3d_gains( sb->get_3d_position(), sound_engine->listener_pos_, sound_engine->listener_forward_, sound_engine->listener_up_, sound_engine->ref_distance_, sound_engine->max_distance_, gl, gr );
			sb->set_mix_gains( gl, gr );
		}
		else
		{
			sb->set_mix_gains( 1.f, 1.f );
		}
	}

	for ( unsigned long n = 0; n < frame_count; n++ )
	{
		float l_value = out[ 0 ];
		float r_value = out[ 1 ];

		for ( auto sb: sound_engine->sound_buffer_list_ )
		{
			// std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );

			if ( ! sb->is_playing() )
			{
				continue;
			}

			float l = 0.f;
			float r = 0.f;
			sb->read_frame( l, r );

			// 3D の音は、頭で求めた左右のゲイン（距離の減衰＋左右の振り分け）を掛ける。3D でない音はゲイン 1
			l_value += l * sb->mix_gain_l();
			r_value += r * sb->mix_gain_r();
		}

		*out++ = l_value * sound_engine->volume_;
		*out++ = r_value * sound_engine->volume_;
	}

	std::ranges::for_each( sound_engine->sound_filter_list_, [ output, frame_count ] ( auto& filter ) { filter->process( static_cast< float* >( output ), frame_count ); } );

	return 0;
}

void SoundEngine::set_volume( float volume )
{
	volume_ = volume;
}

SoundEngine::SoundBuffer* SoundEngine::create_sound_buffer( bool, bool, size_t size, const SoundFormat& format )
{
	auto sound_buffer = new SoundBuffer( *this, format, size );

	std::lock_guard< std::mutex > lock( callback_mutex_ );

	sound_buffer_list_.push_back( sound_buffer );

	return sound_buffer;
}

void SoundEngine::unregister_sound_buffer( SoundBuffer* sound_buffer )
{
	std::lock_guard< std::mutex > lock( callback_mutex_ );

	std::erase( sound_buffer_list_, sound_buffer );
}

void SoundEngine::set_listener_position( const Vector3& p )
{
	listener_pos_staged_ = p;
}

void SoundEngine::set_listener_velocity( const Vector3& )
{
	// ドップラーは非対応（速度は使わない）
}

void SoundEngine::set_listener_orientation( const Vector3& forward, const Vector3& up )
{
	listener_forward_staged_ = forward;
	listener_up_staged_      = up;
}

void SoundEngine::commit()
{
	// staged（game スレッド）→ active（コールバックが読む）を一括 publish。
	std::lock_guard< std::mutex > lock( callback_mutex_ );
	listener_pos_     = listener_pos_staged_;
	listener_forward_ = listener_forward_staged_;
	listener_up_      = listener_up_staged_;
}

void SoundEngine::compute_3d_gains( const Vector3& source, const Vector3& listener_position, const Vector3& listener_forward, const Vector3& listener_up, float ref_distance, float max_distance, float& out_l, float& out_r )
{
	const Vector3 to = source - listener_position;
	const float dist = to.length();

	// 距離減衰: ref 以内は等倍、max 以遠は無音、その間は線形（調整しやすさ優先）。
	float gain;
	if ( dist <= ref_distance )      { gain = 1.f; }
	else if ( dist >= max_distance ) { gain = 0.f; }
	else { gain = ( max_distance - dist ) / ( max_distance - ref_distance ); }

	// パン: リスナーの右ベクトル right = normalize(up × forward) への射影を [-1,1] に。
	// このエンジンの座標では forward=(0,0,1) のとき right=(1,0,0)（移動コードと一致）＝ up×forward。
	// forward×up にすると右が -X になり左右が反転するので注意。
	float pan = 0.f;
	const Vector3& f = listener_forward;
	const Vector3& u = listener_up;
	Vector3 right(
		u.y() * f.z() - u.z() * f.y(),
		u.z() * f.x() - u.x() * f.z(),
		u.x() * f.y() - u.y() * f.x() );
	const float rl = right.length();
	if ( rl > 1e-4f && dist > 1e-4f )
	{
		right /= rl;
		const Vector3 dir = to / dist;
		pan = dir.x() * right.x() + dir.y() * right.y() + dir.z() * right.z();
		pan = ( pan < -1.f ) ? -1.f : ( ( pan > 1.f ) ? 1.f : pan );
	}

	// 等パワーパン（中央 0.707 / 端 1.0、クリップしない）。
	out_l = gain * std::sqrt( ( 1.f - pan ) * 0.5f );
	out_r = gain * std::sqrt( ( 1.f + pan ) * 0.5f );
}

void SoundEngine::add_sound_filter( SoundFilter* filter )
{
	std::lock_guard< std::mutex > lock( callback_mutex_ );

	sound_filter_list_.push_back( filter );
}

void SoundEngine::clear_sound_filter_list()
{
	std::lock_guard< std::mutex > lock( callback_mutex_ );

	sound_filter_list_.clear();
}

} // namespace core::sound::port_audio
