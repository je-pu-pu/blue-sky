#include "SoundManager.h"
#include "Sound.h"
#include "StreamingSound.h"

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
	Sound* sound = new Sound( direct_sound_ );
	sound->load( file_name );

	return sound;
}

}