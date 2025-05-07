#pragma once

#include <core/sound/SoundBuffer.h>
#include <windows.h>

class DirectSoundBuffer;

namespace core::sound::direct_sound
{

/**
 * DirectSound サウンドバッファ
 *
 */
class SoundBuffer : public core::sound::SoundBuffer
{
private:
	std::unique_ptr< DirectSoundBuffer > direct_sound_buffer_;
	void* data_ = nullptr;
	DWORD locked_size_ = 0;

public:
	SoundBuffer( DirectSoundBuffer* );

	void init_3d_sound() override;

	void* lock( size_t, size_t ) override;
	void unlock() override;

	void set_3d_position( const Vector3& ) override;
	void set_3d_velocity( const Vector3& ) override;

	float get_volume() const override;
	void set_volume( float ) override;

	float get_speed() const override;
	void set_speed( float ) override;

	void play( bool ) override;
	bool is_playing() const override;
	void stop() override;

	size_t get_size() const override;
	size_t get_current_position() const override;

}; // class SoundBuffer

} // namespace core::sound
