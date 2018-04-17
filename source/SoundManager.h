#ifndef BLUE_SKY_SOUND_MANAGER_H
#define BLUE_SKY_SOUND_MANAGER_H

#include <game/SoundManager.h>
#include <windows.h>

class DirectSound;
class Vector3;

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

	virtual void set_mute( bool ) override;
	virtual void set_volume( float ) override;

	virtual game::Sound* load( const char*, const char* = 0 ) override;
	virtual game::Sound* load_3d_sound( const char*, const char* = 0 ) override;

	virtual game::Sound* load_music( const char* name, const char* file_name = 0, bool replace = false );

	virtual game::Sound* create_sound( const char*, bool ) override;

	virtual void update() override;

	virtual void stop_all();

	void set_listener_position( const Vector3& );
	void set_listener_velocity( const Vector3& );
	void set_listener_orientation( const Vector3& , const Vector3& );
	void commit();

}; // class SoundManager

} // namespace blue_sky

#endif // BLUE_SKY_SOUND_MANAGER_H
