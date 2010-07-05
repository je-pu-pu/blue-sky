#ifndef BLUE_SKY_SCENE_H
#define BLUE_SKY_SCENE_H

class Direct3D9;

namespace game
{

class Sound;
class Config;

} // namespace game

namespace blue_sky
{

class GameMain;
class GridObjectManager;
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

private:
	const GameMain* game_main_;

protected:
	Direct3D9* direct_3d() const;
	GridObjectManager* grid_object_manager() const;
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

}; // class Scene

} // namespace blue_sky

#endif // BLUE_SKY_SCENE_H