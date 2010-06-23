#include "SoundManager.h"
#include "Sound.h"
#include "StreamingSound.h"
#include "OggVorbisFile.h"
#include "DirectSound.h"

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
	for( SoundMap::iterator i = sound_map().begin(); i != sound_map().end(); ++i )
	{
		i->second->update();
	}
}

}