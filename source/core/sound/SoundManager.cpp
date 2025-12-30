#include "SoundManager.h"
#include "Sound.h"
#include "StreamingSound.h"
#include "OggVorbisFile.h"
#include "SoundEngine.h"
#include "SoundFilter.h"

#include <common/exception.h>

#include <string>

namespace core::sound
{

SoundManager::SoundManager( SoundEngine* sound_engine )
	: sound_engine_( sound_engine )
{

}

SoundManager::~SoundManager()
{
	unload_all();

	sound_engine_->clear_sound_filter_list();
}

void SoundManager::set_mute( bool mute )
{
	game::SoundManager::set_mute( mute );

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
	game::SoundManager::set_volume( v );

	if ( ! is_mute() )
	{
		sound_engine_->set_volume( get_volume() );
	}
}

game::Sound* SoundManager::load( const char* name, const char* file_name )
{
	if ( file_name )
	{
		return game::SoundManager::load( name, ( std::string( "media/sound/" ) + file_name + ".ogg" ).c_str() );
	}

	return game::SoundManager::load( name, ( std::string( "media/sound/" ) + name + ".ogg" ).c_str() );
}

game::Sound* SoundManager::load_3d_sound( const char* name, const char* file_name )
{
	if ( file_name )
	{
		return game::SoundManager::load_3d_sound( name, ( std::string( "media/sound/" ) + file_name + ".ogg" ).c_str() );
	}

	return game::SoundManager::load_3d_sound( name, ( std::string( "media/sound/" ) + name + ".ogg" ).c_str() );
}

game::Sound* SoundManager::load_music( const char* name, const char* file_name, bool replace )
{
	if ( replace )
	{
		unload( name );
	}

	if ( file_name )
	{
		return game::SoundManager::load( name, ( std::string( "media/music/" ) + file_name + ".ogg" ).c_str() );
	}

	return game::SoundManager::load( name, ( std::string( "media/music/" ) + name + ".ogg" ).c_str() );
}

game::Sound* SoundManager::create_sound( const char* file_name, bool is_3d )
{
	auto file = Sound::SoundFile( file_name );
	Sound* sound = nullptr;

	if ( file.size() <= StreamingSound::get_buffer_size() )
	{
		sound = new Sound( sound_engine_ );
	}
	else
	{
		sound = new StreamingSound( sound_engine_ );
	}

	sound->set_3d_sound( is_3d );
	sound->load( file_name );

	return sound;
}

void SoundManager::update()
{
	for( SoundMap::iterator i = sound_map().begin(); i != sound_map().end(); ++i )
	{
		i->second->update();
	}
}

void SoundManager::stop_all()
{
	for ( SoundMap::iterator i = sound_map().begin(); i != sound_map().end(); ++i )
	{
		i->second->stop();
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
