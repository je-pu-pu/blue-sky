#include "SoundManager.h"
#include "Sound.h"
#include "StreamingSound.h"
#include "OggVorbisFile.h"
#include "DirectSound.h"

#include <string>

namespace blue_sky
{

SoundManager::SoundManager( HWND hwnd )
	: direct_sound_( new DirectSound( hwnd ) )
{

}

SoundManager::~SoundManager()
{
	for ( SoundMap::iterator i = sound_map().begin(); i != sound_map().end(); ++i )
	{
		delete i->second;
	}

	delete direct_sound_;
}

game::Sound* SoundManager::load( const char* name, const char* file_name )
{
	if ( file_name )
	{
		return game::SoundManager::load( name, ( std::string( "media/sound/" ) + file_name + ".ogg" ).c_str() );
	}

	return game::SoundManager::load( name, ( std::string( "media/sound/" ) + name + ".ogg" ).c_str() );
}

game::Sound* SoundManager::load_music( const char* name, const char* file_name )
{
	if ( file_name )
	{
		return game::SoundManager::load( name, ( std::string( "media/music/" ) + file_name + ".ogg" ).c_str() );
	}

	return game::SoundManager::load( name, ( std::string( "media/music/" ) + name + ".ogg" ).c_str() );
}

game::Sound* SoundManager::createSound( const char* file_name )
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

	sound->load( file_name );

	return sound;
}

void SoundManager::update()
{
	if ( ! is_enabled() )
	{
		return;
	}

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

}