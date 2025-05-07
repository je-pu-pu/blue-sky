#pragma once

#include <core/type.h>

namespace game
{
	struct SoundFormat;
}

namespace core::sound
{

class SoundBuffer;

/**
 * ƒTƒEƒ“ƒhˆ—‚ğ’ŠÛ‰»
 *
 */
class SoundEngine
{
public:
	using SoundFormat = game::SoundFormat;
	using SoundBuffer = SoundBuffer;

public:
	virtual ~SoundEngine() { }

	virtual void set_volume( float ) = 0;

	virtual SoundBuffer* create_sound_buffer( bool is_3d, bool is_streaming, size_t size, const SoundFormat& format ) const = 0;

	// 3D
	virtual void set_listener_position( const Vector3& ) = 0;
	virtual void set_listener_velocity( const Vector3& ) = 0;
	virtual void set_listener_orientation( const Vector3& , const Vector3& ) = 0;
	virtual void commit() = 0;


}; // class SoundEngine

} // namespace core::sound
