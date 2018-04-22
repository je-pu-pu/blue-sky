#ifndef BLUE_SKY_GAME_PLAY_SCENE_H
#define BLUE_SKY_GAME_PLAY_SCENE_H

#include "Scene.h"

#include <core/Vector3.h>

#include <common/chase_value.h>
#include <common/auto_ptr.h>
#include <common/safe_ptr.h>

#include <map>
#include <list>
#include <vector>
#include <functional>

class Direct3D11FarBillboardsMesh;
class Direct3D11SkyBox;
class Direct3D11ShadowMap;
class Direct3D11Rectangle;
class Direct3D11Axis;

namespace game
{

class Mesh;
class Config;

}; // namespace game

namespace blue_sky
{

class Player;
class Girl;
class Goal;
class ActiveObject;
class Camera;
class DrawingModel;
class DelayedCommand;

struct FrameConstantBufferData;

/**
 * �Q�[���v���C���̏������Ǘ�����
 *
 */
class GamePlayScene : public Scene
{
public:
	typedef game::Mesh					Mesh;
	typedef Direct3D11FarBillboardsMesh	FarBillboardsMesh;
	typedef Direct3D11SkyBox			SkyBox;
	typedef Direct3D11ShadowMap			ShadowMap;
	typedef Direct3D11Rectangle			Rectangle;

	typedef std::function< void( string_t ) > Command;
	typedef std::map< string_t, Command > CommandMap;

	typedef std::function< void() > CommandCall;
	typedef std::list< CommandCall > CommandCallList;
	typedef std::list< DelayedCommand* > DelayedCommandList;

	enum BalloonSoundType
	{
		BALLOON_SOUND_TYPE_NONE = 0,		// �T�E���h�Ȃ�
		BALLOON_SOUND_TYPE_MIX,				// �T�E���h�� BGM �ƃ~�b�N�X
		BALLOON_SOUND_TYPE_SOLO,			// BGM �������ăT�E���h�̂ݍĐ�
		BALLOON_SOUND_TYPE_SCALE,			// BGM �������ĉ��K���Đ�
	};

protected:
	Texture*			ui_texture_;							///< UI �\���p�e�N�X�`��
	bool				is_cleared_;							///< �X�e�[�W�N���A�t���O
	common::auto_ptr< Config >			stage_config_;	

	mutable common::auto_ptr< ShadowMap >		shadow_map_;
	common::auto_ptr< SkyBox >			sky_box_;
	common::auto_ptr< Mesh >			ground_;
	common::auto_ptr< Mesh >			far_billboards_;
	common::auto_ptr< Rectangle >		rectangle_;

	common::auto_ptr< Goal >			goal_;

	common::auto_ptr< Player >			player_;
	common::auto_ptr< Camera >			camera_;

	common::safe_ptr< Girl >			girl_;

	common::auto_ptr< Mesh >			scope_mesh_;			///< �o�ዾ

	Sound*								bgm_;
	common::safe_ptr< Sound >			balloon_bgm_;
	float_t								action_bgm_after_timer_;

	float_t								bpm_;

	/// @todo GraphicsManager �Ɉڍs����
	float_t								drawing_accent_scale_;	///< ( 0 : �A�N�Z���g�����S�ɖ����� ) .. ( 2 : �A�N�Z���g 2 �{ ) ..
	
	common::chase_value< Vector3, float_t >		light_position_;
	common::chase_value< Color, float_t >		ambient_color_;
	common::chase_value< Color, float_t >		shadow_color_;
	common::chase_value< Color, float_t >		shadow_paper_color_;
	bool										shading_enabled_;

	BalloonSoundType					balloon_sound_type_;

	CommandMap							command_map_;
	CommandCallList						stage_setup_command_call_list_;
	DelayedCommandList					delayed_command_list_;

	bool								is_blackout_;
	float_t								blackout_timer_;

	void load_stage_file( const char* );
	void save_stage_file( const char* ) const;

	void load_sound_all( bool );
	void setup_stage();
	
	void setup_command();
	
	// commands
	ActiveObject* create_object_at_player_front( const char_t* );

	void restart();

	void on_goal();

	void set_bpm( const float_t bpm ) { bpm_ = bpm; }
	float_t get_bpm() const { return bpm_; }

	void update_main();
	void update_clear();
	void update_blackout();
	void go_to_next_scene();

	void update_delayed_command();
	void clear_delayed_command();

	void update_balloon_sound();
	void update_shadow();
	
	void exec_command( const string_t& );

	/** @todo �ړ����� */
	void update_render_data_for_frame() const;
	void update_render_data_for_frame_drawing() const;
	void update_render_data_for_frame_for_eye( int ) const;
	void update_render_data_for_object() const;

	void update_frame_constant_buffer_data_sub( FrameConstantBufferData& ) const;

	void render_to_oculus_vr() const;
	void render_to_display() const;

	void render_for_eye( float_t ortho_offset = 0.f ) const;

	void render_text() const;

	void render_shadow_map() const;
	void render_shadow_map( const char*, bool ) const;
	
	void render_sky_box() const;
	void render_far_billboards() const;

	void render_object_skin_mesh() const;
	void render_object_mesh() const;
	void render_object_line() const;
	
	void render_sprite( float_t rotho_offset = 0.f ) const;

	void render_debug_axis() const;
	void render_debug_axis_for_bones( const ActiveObject* ) const;

	/// @todo GraphicsManager �Ɉڍs����
	void render_debug_shadow_map_window() const;

public:
	GamePlayScene( const GameMain* );
	~GamePlayScene();

	void update();				///< ���C�����[�v
	void render();				///< �`��

	bool is_clip_cursor_required() { return true; }

}; // class GamePlayScene

} // namespace blue_sky

#endif // BLUE_SKY_GAME_PLAY_SCENE_H