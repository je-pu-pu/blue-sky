#ifndef BLUE_SKY_SOUND_MANAGER_H
#define BLUE_SKY_SOUND_MANAGER_H

#include <game/SoundManager.h>
#include <windows.h>

class DirectSound;

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

//	virtual game::Sound* load( const char*, const char* );
	virtual game::Sound* createSound( const char* );

	virtual void update();

	virtual void stop_all();

}; // class SoundManager

} // namespace game

#endif // BLUE_SKY_SOUND_MANAGER_H
