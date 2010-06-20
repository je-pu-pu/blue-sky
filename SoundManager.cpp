#include "SoundManager.h"
#include "Sound.h"

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
	Sound* sound = new Sound();
	sound->set_direct_sound_buffer( direct_sound_->load_wave_file( file_name ) );

	return sound;
}

}