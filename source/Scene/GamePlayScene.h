#pragma once

#include "Scene.h"

#include <common/chase_value.h>
#include <common/safe_ptr.h>

#include <list>
#include <unordered_map>
#include <functional>

namespace game
{
	class Mesh;
	class Shader;
	class Config;
};

namespace blue_sky
{
	class Player;
	class Girl;
	class Goal;
	class ActiveObject;
	class Camera;
	class DelayedCommand;

	struct FrameShaderResourceData;

	namespace graphics
	{
		class Rectangle;
	}

/**
 * �Q�[���v���C���̏������Ǘ�����
 *
 */
class GamePlayScene : public Scene
{
public:
	using Mesh					= game::Mesh;
	using Shader				= game::Shader;
	using Rectangle				= graphics::Rectangle;
	
	using Command				= std::function< void( string_t ) >;
	using CommandMap			= std::unordered_map< string_t, Command >;

	using CommandCall			= std::function< void() >;
	using CommandCallList		= std::list< CommandCall >;
	using DelayedCommandList	= std::list< DelayedCommand* >;

	enum BalloonSoundType
	{
		BALLOON_SOUND_TYPE_NONE = 0,		// �T�E���h�Ȃ�
		BALLOON_SOUND_TYPE_MIX,				// �T�E���h�� BGM �ƃ~�b�N�X
		BALLOON_SOUND_TYPE_SOLO,			// BGM �������ăT�E���h�̂ݍĐ�
		BALLOON_SOUND_TYPE_SCALE,			// BGM �������ĉ��K���Đ�
	};

private:
	Texture*							ui_texture_ = nullptr;	///< UI �\���p�e�N�X�`��
	bool								is_cleared_ = false;	///< �X�e�[�W�N���A�t���O
	std::unique_ptr< Config >			stage_config_;	

	Shader*								debug_texture_shader_ = nullptr;

	Player*								player_ = nullptr;
	std::unique_ptr< Camera >			camera_;				///< @todo GameObjectManager �ŊǗ����ĎQ�Ƃ��邾���ɂ���
	Girl*								girl_ = nullptr;					
	Goal*								goal_ = nullptr;

	/// @todo ��������
	Model*								far_billboards_ = nullptr;	///< ���i�r���{�[�h
	Model*								scope_mesh_ = nullptr;		///< �o�ዾ
	Model*								rectangle_ = nullptr;

	Sound*								bgm_ = nullptr;
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

protected:
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

	void update_frame_constant_buffer_data_sub( FrameShaderResourceData& ) const;

	void render_to_oculus_vr() const;
	void render_to_display() const;

	void render_for_eye( float_t ortho_offset = 0.f ) const;

	void render_text() const;

	void render_sky_box() const;
	void render_far_billboards() const;
	
	void render_sprite( float_t rotho_offset = 0.f ) const;

	void render_debug_axis() const;
	void render_debug_axis_for_bones( const ActiveObject* ) const;

	/// @todo GraphicsManager �Ɉڍs����
	void render_debug_shadow_map_window() const;

public:
	explicit GamePlayScene( const GameMain* );
	~GamePlayScene();

	void update() override;				///< ���C�����[�v
	void render() override;				///< �`��

	bool is_clip_cursor_required() override { return true; }

}; // class GamePlayScene

} // namespace blue_sky
