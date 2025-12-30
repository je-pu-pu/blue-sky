#pragma once


#include <core/type.h>
#include <game/SoundManager.h>

#include <windows.h>

namespace game
{

class Sound;
struct SoundFormat;

}

namespace core::sound
{

class SoundEngine;
class SoundBuffer;
class SoundFilter;

/**
 * サウンド管理クラス
 *
 */
class SoundManager : public game::SoundManager
{
public:
	using SoundFormat = game::SoundFormat;

private:
	SoundEngine* sound_engine_;
	std::vector< std::unique_ptr< SoundFilter > > sound_filter_list_;

public:
	SoundManager( SoundEngine* );
	virtual ~SoundManager();

	SoundEngine* get_sound_engine() const { return sound_engine_; }

	virtual void set_mute( bool ) override;
	virtual void set_volume( float ) override;

	virtual game::Sound* load( const char*, const char* = 0 ) override;
	virtual game::Sound* load_3d_sound( const char*, const char* = 0 ) override;

	virtual game::Sound* load_music( const char* name, const char* file_name = 0, bool replace = false );

	virtual game::Sound* create_sound( const char*, bool ) override;

	virtual void update() override;

	virtual void stop_all();

	void add_sound_filter( SoundFilter* filter );

	void set_listener_position( const Vector3& );
	void set_listener_velocity( const Vector3& );
	void set_listener_orientation( const Vector3& , const Vector3& );
	void commit();

	const SoundFormat& get_format() const;

}; // class SoundManager

} // namespace core::sound
