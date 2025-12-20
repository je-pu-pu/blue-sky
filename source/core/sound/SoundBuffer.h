#pragma once

#include <core/type.h>

namespace core::sound
{

/**
 * 抽象サウンドバッファ
 *
 */
class SoundBuffer
{
private:

public:
	virtual ~SoundBuffer() { };

	virtual void init_3d_sound() = 0;

	virtual void* lock( size_t = 0, size_t = 0 ) = 0;
	virtual void unlock() = 0;

	virtual void set_3d_position( const Vector3& ) = 0;
	virtual void set_3d_velocity( const Vector3& ) = 0;

	virtual float get_volume() const = 0;
	virtual void set_volume( float ) = 0;

	virtual float get_speed() const = 0;
	virtual void set_speed( float ) = 0;

	virtual void play( bool ) = 0;
	virtual bool is_playing() const = 0;
	virtual void stop() = 0;

	virtual size_t get_size() const = 0;
	virtual size_t get_current_position() const = 0;

}; // class SoundBuffer

} // namespace core::sound
