#include "SoundManager.h"
#include "Sound.h"
#include "StreamingSound.h"
#include "OggVorbisFile.h"
#include <core/sound/DirectSound/DirectSound.h>
#include <core/sound/DirectSound/DirectSoundBuffer.h>
#include <core/DirectX.h>

#include <common/exception.h>

#include <string>

namespace core
{

SoundManager::SoundManager( HWND hwnd )
	: direct_sound_( new DirectSound( hwnd ) )
{

}

SoundManager::~SoundManager()
{
	unload_all();

	delete direct_sound_;
}

void SoundManager::set_mute( bool mute )
{
	game::SoundManager::set_mute( mute );

	if ( is_mute() )
	{
		DIRECT_X_FAIL_CHECK( direct_sound_->get_primary_buffer()->SetVolume( DSBVOLUME_MIN ) );
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
		LONG volume = static_cast< long >( ( get_volume() - Sound::VOLUME_MIN ) / ( Sound::VOLUME_MAX - Sound::VOLUME_MIN ) * ( DSBVOLUME_MAX - DSBVOLUME_MIN ) + DSBVOLUME_MIN );
		DIRECT_X_FAIL_CHECK( direct_sound_->get_primary_buffer()->SetVolume( volume ) );
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
	Sound::SoundFile* file = new Sound::SoundFile( file_name );
	Sound* sound = 0;

	if ( file->size() <= StreamingSound::get_buffer_size() )
	{
		sound = new Sound( direct_sound_ );
	}
	else
	{
		sound = new StreamingSound( direct_sound_ );
	}

	delete file;

	sound->set_3d_sound( is_3d );
	sound->load( file_name );

	if ( is_3d )
	{
		DIRECT_X_FAIL_CHECK( sound->get_direct_sound_buffer()->get_direct_sound_3d_buffer()->SetMinDistance( 2.f, DS3D_DEFERRED ) );
		DIRECT_X_FAIL_CHECK( sound->get_direct_sound_buffer()->get_direct_sound_3d_buffer()->SetMaxDistance( 50.f, DS3D_DEFERRED ) );
		DIRECT_X_FAIL_CHECK( sound->get_direct_sound_buffer()->get_direct_sound_3d_buffer()->SetMode( DS3DMODE_NORMAL, DS3D_DEFERRED ) );
	}

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

void SoundManager::set_listener_position( const Vector3& p )
{
	direct_sound_->get_listener()->SetPosition( p.x(), p.y(), p.z(), DS3D_DEFERRED );
}

void SoundManager::set_listener_velocity( const Vector3& v )
{
	direct_sound_->get_listener()->SetVelocity( v.x(), v.y(), v.z(), DS3D_DEFERRED );
}

void SoundManager::set_listener_orientation( const Vector3& front, const Vector3& top )
{
	direct_sound_->get_listener()->SetOrientation( front.x(), front.y(), front.z(), top.x(), top.y(), top.z(), DS3D_DEFERRED );
}

void SoundManager::commit()
{
	direct_sound_->get_listener()->CommitDeferredSettings();
}

}
