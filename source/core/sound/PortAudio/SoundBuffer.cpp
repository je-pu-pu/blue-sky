#include "SoundBuffer.h"
#include "SoundEngine.h"
#include <limits>

namespace core::sound::port_audio
{

SoundBuffer::SoundBuffer( SoundEngine& sound_engine, const SoundFormat& format, size_t size )
	: sound_engine_( sound_engine )
	, format_( format )
	, data_( size )
	, rate_stepper_( format.sampling_rate, sound_engine.get_format().sampling_rate )
{

}

SoundBuffer::~SoundBuffer()
{
	sound_engine_.unregister_sound_buffer( this );
}

void SoundBuffer::init_3d_sound()
{
	is_3d_.store( true, std::memory_order_relaxed );
}

void* SoundBuffer::lock( size_t offset, size_t )
{
	return & data_[ offset ];
}

void SoundBuffer::unlock()
{
	// 書き込まれたデータはそのまま持つ。出力と周波数が違う音は read_frame() が読む速さで合わせる
	// （以前はここで変換する予定のまま何もしておらず、22050 Hz の音楽が 44100 Hz の出力で倍の速さで鳴っていた）
}

void SoundBuffer::set_3d_position( const Vector3& p )
{
	pos_x_.store( p.x(), std::memory_order_relaxed );
	pos_y_.store( p.y(), std::memory_order_relaxed );
	pos_z_.store( p.z(), std::memory_order_relaxed );
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
	rate_stepper_.reset();
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

float SoundBuffer::sample_at( size_t position ) const
{
	if ( position + sizeof( s16_t ) > data_.size() )
	{
		return 0.f;
	}

	return static_cast< float >( * reinterpret_cast< const s16_t* >( & data_[ position ] ) ) / std::numeric_limits< s16_t >::max();
}

void SoundBuffer::read_frame( float& left, float& right )
{
	const size_t channels = format_.channels == 2 ? 2 : 1;
	const size_t frame_bytes = sizeof( s16_t ) * channels;

	left = sample_at( current_position_ );
	right = channels == 2 ? sample_at( current_position_ + sizeof( s16_t ) ) : left;

	for ( size_t n = rate_stepper_.step(); n > 0 && is_playing_; n-- )
	{
		current_position_ += frame_bytes;

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
	}
}

} // namespace core::sound::port_audio
