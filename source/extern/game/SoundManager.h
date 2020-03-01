#pragma once

#include <type/type.h>
#include <unordered_map>

namespace game
{

class Sound;

/**
 * サウンド管理クラス
 *
 * @todo core/sound/SoundManager に統合する
 */
class SoundManager
{
public:
	using SoundMap			= std::unordered_map< string_t, Sound* >;
	using GroupedSoundMap	= std::unordered_map< string_t, SoundMap >;

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
	virtual void set_mute( bool is_mute );

	float get_volume() const { return volume_; }
	virtual void set_volume( float volume );

	virtual void push_group( const char_t* group_name );
	virtual void pop_group();

	virtual Sound* load( const char* name, const char* file_path );
	virtual Sound* load_3d_sound( const char* name, const char* file_path );

	virtual void unload( const char* name );
	virtual void unload_all();

	virtual Sound* create_sound( const char* file_path, bool is_3d ) = 0;

	virtual Sound* get_sound( const char* name ) const;

	virtual void update() = 0;

}; // class SoundManager

} // namespace game
