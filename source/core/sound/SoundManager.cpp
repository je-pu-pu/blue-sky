#include "SoundManager.h"
#include "Sound.h"
#include "StreamingSound.h"
#include "OggVorbisFile.h"
#include "SoundEngine.h"
#include "SoundFilter.h"

#include <common/exception.h>

#include <cassert>
#include <chrono>
#include <string>

namespace core::sound
{

SoundManager::SoundManager( SoundEngine* sound_engine )
	: sound_engine_( sound_engine )
{

}

SoundManager::~SoundManager()
{
	stop_update_thread();
	unload_all();

	sound_engine_->clear_sound_filter_list();
}

void SoundManager::set_mute( bool mute )
{
	is_mute_ = mute;

	if ( is_mute() )
	{
		sound_engine_->set_volume( Sound::VOLUME_MIN );
	}
	else
	{
		set_volume( get_volume() );
	}
}

void SoundManager::set_volume( float v )
{
	volume_ = v;

	if ( ! is_mute() )
	{
		sound_engine_->set_volume( get_volume() );
	}
}

SoundManager::Sound* SoundManager::load( const char* name, const char* file_name )
{
	return load_from_path( name, std::string( "media/sound/" ) + ( file_name ? file_name : name ) + ".ogg", false );
}

SoundManager::Sound* SoundManager::load_3d_sound( const char* name, const char* file_name )
{
	return load_from_path( name, std::string( "media/sound/" ) + ( file_name ? file_name : name ) + ".ogg", true );
}

SoundManager::Sound* SoundManager::load_music( const char* name, const char* file_name, bool replace )
{
	return load_from_path( name, std::string( "media/music/" ) + ( file_name ? file_name : name ) + ".ogg", false, replace );
}

SoundManager::Sound* SoundManager::reload( const char* name, const char* file_name )
{
	return load_from_path( name, std::string( "media/sound/" ) + ( file_name ? file_name : name ) + ".ogg", false, true );
}

SoundManager::Sound* SoundManager::load_from_path( const char* name, const string_t& file_path, bool is_3d, bool force_reload )
{
	std::lock_guard< std::recursive_mutex > lock( mutex_ );
	const SoundHandle handle = sounds_.get_handle( name );
	Sound* current = sounds_.resolve( handle );

	if ( current && ! force_reload && std::string( current->get_file_name() ) == file_path && current->is_3d_sound() == is_3d )
	{
		return current;
	}

	// 小さいファイルは全部読み込み、大きいファイルは少しずつ読む
	auto file = Sound::SoundFile( file_path.c_str() );
	std::unique_ptr< Sound > sound( file.size() <= StreamingSound::get_buffer_size() ? new Sound( sound_engine_ ) : new StreamingSound( sound_engine_ ) );

	sound->set_3d_sound( is_3d );
	sound->load( file_path.c_str() );
	sound->set_name( name );
	sound->set_file_name( file_path.c_str() );

	Sound* loaded = sound.get();

	if ( current )
	{
		// 番号はそのまま、中身を差し替えて古い音を解放する（以前は先に古い音を消してから作り直し、ハンドルの番号も変わった）
		sounds_.replace( handle, std::move( sound ) );
	}
	else
	{
		sounds_.add_named( name, sound.release() );
	}

	grouped_sound_map_[ group_name_ ].insert( name );

	return loaded;
}

void SoundManager::push_group( const char_t* group_name )
{
	assert( group_name_.empty() );
	group_name_ = group_name;
}

void SoundManager::pop_group()
{
	std::lock_guard< std::recursive_mutex > lock( mutex_ );
	if ( group_name_.empty() )
	{
		return;
	}

	auto i = grouped_sound_map_.find( group_name_ );
	group_name_.clear();

	if ( i == grouped_sound_map_.end() )
	{
		return;
	}

	for ( const auto& name : i->second )
	{
		unload( name.c_str() );		// 外し済みのこともある（そのときは何もしない）
	}

	grouped_sound_map_.erase( i );
}

void SoundManager::unload( const char* name )
{
	std::lock_guard< std::recursive_mutex > lock( mutex_ );
	sounds_.remove_named( name );		// 音は名前 1 つにだけ付けるので、ここで解放される
}

void SoundManager::unload_all()
{
	std::lock_guard< std::recursive_mutex > lock( mutex_ );
	sounds_.clear_all();
	grouped_sound_map_.clear();
}

std::size_t SoundManager::flush_releases()
{
	std::lock_guard< std::recursive_mutex > lock( mutex_ );
	return sounds_.flush_releases();
}

SoundManager::Sound* SoundManager::get_sound( const char* name ) const
{
	std::lock_guard< std::recursive_mutex > lock( mutex_ );
	return sounds_.resolve( sounds_.get_handle( name ) );
}

SoundManager::SoundHandle SoundManager::get_sound_handle( const char* name ) const
{
	std::lock_guard< std::recursive_mutex > lock( mutex_ );
	return sounds_.get_handle( name );
}

SoundManager::Sound* SoundManager::resolve( SoundHandle handle ) const
{
	std::lock_guard< std::recursive_mutex > lock( mutex_ );
	return sounds_.resolve( handle );
}

void SoundManager::update()
{
	std::lock_guard< std::recursive_mutex > lock( mutex_ );
	for ( const auto& sound : sounds_.get_resource_list() )
	{
		sound->update();
	}
}

void SoundManager::stop_all()
{
	std::lock_guard< std::recursive_mutex > lock( mutex_ );
	for ( const auto& sound : sounds_.get_resource_list() )
	{
		sound->stop();
	}
}

void SoundManager::start_update_thread( unsigned int interval_msec )
{
	if ( update_thread_running_ )
	{
		return;
	}

	update_thread_running_ = true;
	update_thread_ = std::thread( [this, interval_msec]
	{
		while ( update_thread_running_ )
		{
			update();
			std::this_thread::sleep_for( std::chrono::milliseconds( interval_msec ) );
		}
	} );
}

void SoundManager::stop_update_thread()
{
	update_thread_running_ = false;

	if ( update_thread_.joinable() )
	{
		update_thread_.join();
	}
}

void SoundManager::add_sound_filter( SoundFilter* filter )
{
	sound_filter_list_.emplace_back( filter );

	sound_engine_->add_sound_filter( filter );
}

void SoundManager::set_listener_position( const Vector3& p )
{
	sound_engine_->set_listener_position( p );
}

void SoundManager::set_listener_velocity( const Vector3& v )
{
	sound_engine_->set_listener_velocity( v );
}

void SoundManager::set_listener_orientation( const Vector3& front, const Vector3& top )
{
	sound_engine_->set_listener_orientation( front, top );
}

void SoundManager::commit()
{
	sound_engine_->commit();
}

const SoundManager::SoundFormat& SoundManager::get_format() const
{
	return sound_engine_->get_format();
}

} // namespace core::sound
