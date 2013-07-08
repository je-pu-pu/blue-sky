//**********************************************************
//! copyright (c) JE all rights reserved
//!
//! \file		SoundManager.cpp
//! \date		2008/09/21
//! \author		JE
//! \brief		SoundManager ‚ÌŽÀ‘•
//**********************************************************

#include "SoundManager.h"
#include "Sound.h"

#include <common/exception.h>

namespace game
{

SoundManager::SoundManager()
	: is_mute_( false )
	, volume_( 1.f )
{

}

SoundManager::~SoundManager()
{

}

void SoundManager::set_mute( bool mute )
{
	is_mute_ = mute;
}

void SoundManager::set_volume( float volume )
{
	volume_ = volume;
}

Sound* SoundManager::load( const char* name, const char* file_name )
{
	return load_common( name, file_name, false );
}

Sound* SoundManager::load_3d_sound( const char* name, const char* file_name )
{
	return load_common( name, file_name, true );
}

Sound* SoundManager::load_common( const char* name, const char* file_name, bool is_3d )
{
	SoundMap::const_iterator i = sound_map_.find( name  );

	if ( i != sound_map_.end() )
	{
		if ( std::string( i->second->get_file_name() ) == file_name && i->second->is_3d_sound() == is_3d )
		{
			return sound_map_[ name ];
		}

		unload( name );

		// COMMON_THROW_EXCEPTION_MESSAGE( "sound conflict." );
	}

	Sound* sound = create_sound( file_name, is_3d );
	sound->set_name( name );
	sound->set_file_name( file_name );
	
	sound_map_[ name ] = sound;

	return sound;
}

void SoundManager::unload( const char* name )
{
	SoundMap::const_iterator i = sound_map_.find( name );

	if ( i != sound_map_.end() )
	{
		delete i->second;

		sound_map_.erase( i );
	}
}

void SoundManager::unload_all()
{
	for ( SoundMap::iterator i = sound_map_.begin(); i != sound_map_.end(); ++i )
	{
		delete i->second;
	}

	sound_map().clear();
}

Sound* SoundManager::get_sound( const char* name ) const
{
	SoundMap::const_iterator i = sound_map_.find( name );

	if ( i == sound_map_.end() )
	{
		return 0;
	}

	return i->second;
}

}