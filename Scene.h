#ifndef BLUE_SKY_SCENE_H
#define BLUE_SKY_SCENE_H

#include <string>
#include <d3dx9.h>

class Direct3D9;

namespace game
{

class Sound;
class Config;

} // namespace game

namespace blue_sky
{

class GameMain;
class GridDataManager;
class GridObjectManager;
class ActiveObjectManager;
class SoundManager;
class Input;
class Config;

/**
 * 場面ごとの処理、描画を管理する
 *
 */
class Scene
{
public:
	typedef game::Sound Sound;
	typedef game::Config Config;
	typedef IDirect3DTexture9 Texture;

private:
	const GameMain* game_main_;
	std::string next_scene_;

protected:
	Direct3D9* direct_3d() const;
	GridDataManager* grid_data_manager() const;
	GridObjectManager* grid_object_manager() const;
	ActiveObjectManager* active_object_manager() const;
	SoundManager* sound_manager() const;
	Input* input() const;
	Config* config() const;

	int get_width() const;
	int get_height() const;

public:
	Scene( const GameMain* );
	virtual ~Scene();

	virtual void update() = 0;				///< メインループ
	virtual void render() = 0;				///< 描画

	bool is_first_game_play() const;
	unsigned int get_current_time() const;

	const std::string& get_next_scene() const { return next_scene_; }
	void set_next_scene( const std::string& next_scene ) { next_scene_ = next_scene; }

}; // class Scene

} // namespace blue_sky

#endif // BLUE_SKY_SCENE_H