#include "SoundEngine.h"
#include "SoundBuffer.h"
#include <core/sound/Sound.h>
#include <core/type.h>

#include <game/SoundFormat.h>
#include <common/exception.h>

#include <portaudio/pa_win_wasapi.h>


namespace core::sound::port_audio
{

SoundEngine::SoundEngine()
{
	PaError e = Pa_Initialize();

	if ( e != paNoError )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( string_t( "Pa_Initialize() failed.\n" ) + Pa_GetErrorText( e ) );
	}

	auto info = Pa_GetDeviceInfo( Pa_GetDefaultOutputDevice() );

	const auto channel_count = 2;
	const auto sample_format = paFloat32;
	const auto sample_rate = 44100; // info->defaultSampleRate; // 44100;

	auto wasapi_streawm_info = PaWasapiStreamInfo{
		.size = sizeof( PaWasapiStreamInfo ),
		.hostApiType = paWASAPI,
		.version = 1,
		.flags = paWinWasapiAutoConvert,
	};

	const auto output_params = PaStreamParameters{
		.device = Pa_GetDefaultOutputDevice(),
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

	format_ = {
		.channels = channel_count,
		.sampling_rate = static_cast< int >( sample_rate ),
		.bit_depth = 32
	};
}

SoundEngine::~SoundEngine()
{
	Pa_StopStream( stream_ );

	Pa_CloseStream( stream_ );

	Pa_Terminate();
}

int SoundEngine::callback( const void* input, void* output, unsigned long frame_count, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void* user_data )
{
	auto sound_engine = static_cast< SoundEngine* >( user_data );
	auto out = static_cast< float* >( output );

	for ( unsigned long n = 0; n < frame_count; n++ )
	{
		float l_value = 0.f;
		float r_value = 0.f;

		for ( auto sb: sound_engine->sound_buffer_list_ )
		{
			if ( ! sb->is_playing() )
			{
				continue;
			}
			
			if ( sb->get_format().channels == 2 )
			{
				l_value += sb->read();
				r_value += sb->read();
			}
			else if ( sb->get_format().channels == 1 )
			{
				const auto value = sb->read();
				l_value += value;
				r_value += value;
			}
		}

		*out++ = l_value;
		*out++ = r_value;
	}

	return 0;
}

void SoundEngine::set_volume( float volume )
{
	
}

SoundEngine::SoundBuffer* SoundEngine::create_sound_buffer( bool is_3d_sound, bool is_streaming, size_t size, const SoundFormat& format )
{
	auto sound_buffer = new SoundBuffer( *this, format, size );

	sound_buffer_list_.push_back( sound_buffer );

	return sound_buffer;
}

void SoundEngine::unregister_sound_buffer( SoundBuffer* sound_buffer )
{
	std::erase( sound_buffer_list_, sound_buffer );
}

void SoundEngine::set_listener_position( const Vector3& )
{
	
}

void SoundEngine::set_listener_velocity( const Vector3& )
{
	
}

void SoundEngine::set_listener_orientation( const Vector3&, const Vector3& )
{
	
}

void SoundEngine::commit()
{
	
}

} // namespace core::sound::port_audio
