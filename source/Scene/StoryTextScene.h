#pragma once

#include "Scene.h"
#include <common/chase_value.h>
#include <list>
#include <memory>

namespace core
{
	class BgSpriteLayer;
}

namespace blue_sky
{

/**
 * ストーリー画面の処理を管理する
 *
 */
class StoryTextScene : public Scene
{
public:
	static inline const char_t* name = "story_text";

	using BgSpriteLayer		= core::BgSpriteLayer;
	using BgSpriteLayerList	= std::list< std::unique_ptr< BgSpriteLayer > >;

private:
	std::wstring	text_;
	float			text_y_;
	float			text_y_target_;
	float			text_scroll_speed_;
	Color			text_color_;
	Color			text_border_color_;
	
	std::string		next_scene_name_;
	std::string		next_stage_name_;

	// リソースはハンドルで持ち、使うたびに引く（以前は生ポインタで、解放・差し替えの後に古いものを指し得た）
	TextureHandle	sprite_texture_;
	
	BgSpriteLayerList	bg_sprite_layer_list_;

	SoundHandle		bgm_;		///< 空なら BGM の指定が無い
	SoundHandle		sound_;		///< 空なら音声の指定が無い

	bool			is_skipped_;

	void load_story_text_file( const char* );

	/// 背景の画像を読み、レイヤーを作る
	std::unique_ptr< BgSpriteLayer > create_bg_sprite_layer( const char_t* layer_name, const char_t* texture_name, const char_t* file_name ) const;

	/// 読んだ「始め・目標・速さ」を重なりの動きに当てる（書いていない所は重なりの既定の値のまま）
	template< typename T, typename Desc, typename Set >
	static void apply_chase( common::chase_value< T >& chase, const Desc& desc, Set set )
	{
		if ( desc.from ) { set( chase.value(), * desc.from ); }
		if ( desc.to ) { set( chase.target_value(), * desc.to ); }
		if ( desc.speed ) { set( chase.speed(), * desc.speed ); }
	}

public:
	StoryTextScene( GameMain& game_main, const char*, const char* );
	~StoryTextScene();

	void update() override;				///< メインループ
	void render() override;				///< 描画

}; // class StoryTextScene

} // namespace blue_sky
