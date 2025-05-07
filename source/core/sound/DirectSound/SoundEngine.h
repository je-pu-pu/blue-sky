#pragma once

#include "SoundBuffer.h"
#include <core/sound/SoundEngine.h>

#define DIRECTSOUND_VERSION 0x1000
#include <dsound.h>

#include <memory>

class DirectSound;

namespace core::sound::direct_sound
{

/**
 * DirectSound ‚ðŽg‚Á‚½ SoundEngine
 *
 */
class SoundEngine : public core::sound::SoundEngine
{
public:
	using SoundBuffer = direct_sound::SoundBuffer;

private:
	std::unique_ptr< DirectSound > direct_sound_;

public:
	SoundEngine( HWND );
	~SoundEngine();

	void set_volume( float ) override;

	SoundBuffer* create_sound_buffer( bool is_3d, bool is_streaming, size_t size, const SoundFormat& format ) const override;

	// 3D
	void set_listener_position( const Vector3& ) override;
	void set_listener_velocity( const Vector3& ) override;
	void set_listener_orientation( const Vector3& , const Vector3& ) override;
	void commit() override;

}; // class SoundEngine

} // namespace core::sound::direct_sound
