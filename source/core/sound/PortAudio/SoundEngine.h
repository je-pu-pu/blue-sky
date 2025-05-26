#pragma once

#include <core/sound/SoundEngine.h>
#include "SoundBuffer.h"
#include <game/SoundFormat.h>
#include <portaudio/portaudio.h>
#include <mutex>

namespace core::sound
{
	class SoundFilter;
}

namespace core::sound::port_audio
{

/**
 * PortAudio ‚ðŽg‚Á‚½ SoundEngine
 *
 */
class SoundEngine : public core::sound::SoundEngine
{
public:
	using SoundBuffer = port_audio::SoundBuffer;

private:
	PaStream* stream_ = nullptr;
	std::vector< SoundBuffer* > sound_buffer_list_;
	SoundFormat format_;

	std::mutex callback_mutex_;

	std::vector< SoundFilter* > sound_filter_list_;

	static int callback( const void* input, void* output, unsigned long frame_count, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void* );

public:
	SoundEngine();
	~SoundEngine();

	void set_volume( float ) override;

	SoundBuffer* create_sound_buffer( bool is_3d, bool is_streaming, size_t size, const SoundFormat& format ) override;
	void unregister_sound_buffer( SoundBuffer* );

	// 3D
	void set_listener_position( const Vector3& ) override;
	void set_listener_velocity( const Vector3& ) override;
	void set_listener_orientation( const Vector3& , const Vector3& ) override;
	void commit() override;

	const SoundFormat& get_format() const override { return format_; }

	void add_sound_filter( SoundFilter* ) override;
	void clear_sound_filter_list() override;

}; // class SoundEngine

} // namespace core::sound::port_audio
