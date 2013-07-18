#ifndef GAME_SOUND_MANAGER_H
#define GAME_SOUND_MANAGER_H

#include <map>
#include <list>
#include <type/type.h>

namespace game
{

class Sound;

/**
 * サウンド管理クラス
 *
 */
class SoundManager
{
public:
	typedef std::map< string_t, Sound* > SoundMap;
	typedef std::map< string_t, SoundMap > GroupedSoundMap;

private:
	SoundMap sound_map_;
	
	string_t group_name_;
	GroupedSoundMap grouped_sound_map_;

	bool is_mute_;
	float volume_;

protected:
	SoundMap& sound_map() { return sound_map_; };
	const SoundMap& sound_map() const { return sound_map_; };

	virtual Sound* load_common( const char*, const char*, bool );
public:
	SoundManager();
	virtual ~SoundManager();

	bool is_mute() const { return is_mute_; }
	virtual void set_mute( bool );

	float get_volume() const { return volume_; }
	virtual void set_volume( float v );

	virtual void push_group( const char_t* );
	virtual void pop_group();

	virtual Sound* load( const char*, const char* );
	virtual Sound* load_3d_sound( const char*, const char* );

	virtual void unload( const char* );
	virtual void unload_all();

	virtual Sound* create_sound( const char*, bool ) = 0;

	virtual Sound* get_sound( const char* ) const;

	virtual void update() = 0;

}; // class SoundManager

} // namespace game

#endif // GAME_SOUND_MANAGER_H
