#ifndef BLUE_SKY_SCENE_H
#define BLUE_SKY_SCENE_H

#include <string>
#include <d3d11.h>

class Direct3D11;

namespace game
{

class Sound;
class Config;

} // namespace game

namespace blue_sky
{

class GameMain;
class ActiveObjectManager;
class DrawingModelManager;
class SoundManager;
class Input;

/**
 * 場面ごとの処理、描画を管理する
 *
 */
class Scene
{
public:
	typedef game::Sound					Sound;
	typedef game::Config				Config;
	
	typedef Direct3D11					Direct3D;
	typedef ID3D11ShaderResourceView	Texture;

private:
	const GameMain* game_main_;
	std::string name_;
	std::string next_scene_;
	std::string next_stage_name_;

protected:
	Direct3D* direct_3d() const;
	ActiveObjectManager* active_object_manager() const;
	DrawingModelManager* drawing_model_manager() const;

	SoundManager* sound_manager() const;
	Input* input() const;
	Config* config() const;
	Config* save_data() const;

	const GameMain* get_game_main() const { return game_main_; }

	int get_width() const;
	int get_height() const;

public:
	Scene( const GameMain* );
	virtual ~Scene();

	void set_name( const std::string name ) { name_ = name; }
	const std::string& get_name() const { return name_; }

	virtual void update() = 0;				///< メインループ
	virtual bool render() = 0;				///< 描画

	bool is_first_game_play() const;
	unsigned int get_current_time() const;

	const std::string& get_next_scene() const { return next_scene_; }
	void set_next_scene( const std::string& next_scene ) { next_scene_ = next_scene; }

	const std::string& get_stage_name() const;
	
	const std::string& get_next_stage_name() const { return next_stage_name_; }
	void set_next_stage_name( const std::string& next_stage_name ) { next_stage_name_ = next_stage_name; }

	virtual bool is_clip_cursor_required() { return false; }

}; // class Scene

} // namespace blue_sky

#endif // BLUE_SKY_SCENE_H