#include "SoundBuffer.h"
#include "SoundEngine.h"
#include <limits>

namespace core::sound::port_audio
{

SoundBuffer::SoundBuffer( SoundEngine& sound_engine, const SoundFormat& format, size_t size )
	: sound_engine_( sound_engine )
	, format_( format )
	, data_( size )
{

}

SoundBuffer::~SoundBuffer()
{
	sound_engine_.unregister_sound_buffer( this );
}

void SoundBuffer::init_3d_sound()
{
	
}

void* SoundBuffer::lock( size_t offset, size_t )
{
	return & data_[ offset ];
}

void SoundBuffer::unlock()
{
	/// @todo 効率化する

	// auto old_data = data_;

	// data_.reserve( data_.size() * 2 );

	/// 変換後・変換前の周波数の比と型から dst のサイズを求める
	
	/// 変換を行う
}

void SoundBuffer::set_3d_position( const Vector3& )
{
	
}

void SoundBuffer::set_3d_velocity( const Vector3& )
{
	
}

float SoundBuffer::get_volume() const
{
	return volume_;
}

void SoundBuffer::set_volume( float v )
{
	volume_ = v;
}

float SoundBuffer::get_speed() const
{
	return 1.f;
}

void SoundBuffer::set_speed( float )
{
	
}

void SoundBuffer::play( bool loop )
{
	current_position_ = 0;
	is_playing_ = true;
	is_looping_ = loop;
}

bool SoundBuffer::is_playing() const
{
	return is_playing_ ;
}

void SoundBuffer::stop()
{
	is_playing_ = false;
}

size_t SoundBuffer::get_size() const
{
	return data_.size();
}

size_t SoundBuffer::get_current_position() const
{
	return current_position_;
}

float SoundBuffer::read()
{
	float value = static_cast< float >( * reinterpret_cast< s16_t* >( & data_[ current_position_ ] ) ) / std::numeric_limits< s16_t >::max();

	current_position_ += 2;

	if ( current_position_ >= data_.size() )
	{
		if ( is_looping_ )
		{
			current_position_ = 0;
		}
		else
		{
			is_playing_ = false;
		}
	}

	return value;
}

} // namespace core::sound::port_audio
