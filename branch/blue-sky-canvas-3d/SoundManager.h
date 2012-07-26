#ifndef BLUE_SKY_SOUND_MANAGER_H
#define BLUE_SKY_SOUND_MANAGER_H

#include <game/SoundManager.h>
#include <windows.h>

class DirectSound;
class vector3;

namespace game
{

class Sound;

}

namespace blue_sky
{

/**
 *サウンド管理クラス
 *
 */
class SoundManager : public game::SoundManager
{
private:
	DirectSound* direct_sound_;

public:
	SoundManager( HWND );
	virtual ~SoundManager();

	virtual void set_mute( bool );
	virtual void set_volume( float );

	virtual game::Sound* load( const char*, const char* = 0 );
	virtual game::Sound* load_3d_sound( const char*, const char* = 0 );

	virtual game::Sound* load_music( const char*, const char* = 0 );

	virtual game::Sound* create_sound( const char*, bool );

	virtual void update();

	virtual void stop_all();

	void set_listener_position( const vector3& );
	void set_listener_velocity( const vector3& );
	void set_listener_orientation( const vector3& , const vector3& );
	void commit();

}; // class SoundManager

} // namespace blue_sky

#endif // BLUE_SKY_SOUND_MANAGER_H
