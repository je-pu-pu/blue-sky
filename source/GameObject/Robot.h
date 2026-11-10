#pragma once

#include "ActiveObject.h"

#include <vector>
#include <list>

namespace blue_sky
{

class Player;

namespace graphics { class GraphicsManager; }

/**
 * 敵
 *
 */
class Robot : public ActiveObject
{
public:
	using VectorArray	= std::vector< Vector >;

	enum class Mode
	{
		STAND = 0,
		ROTATION,
		PATROL,
		ATTENTION,
		FIND,
		CHASE,
		SHUTDOWN,
		FLOAT,
		// DETOUR
	};

private:
	const Player*	player_;
	Texture*		texture_;		///< テクスチャ（今の状態のもの）

	/// 状態ごとのテクスチャ（コンストラクタで 1 回だけ引く。以前は状態が変わるたびに GameMain から名前で引き直していた）
	Texture*		texture_normal_ = nullptr;
	Texture*		texture_warn_ = nullptr;
	Texture*		texture_error_ = nullptr;
	Texture*		texture_tender_ = nullptr;

	Mode			mode_;			///< 現在の動作モード
	Mode			mode_backup_;	///< 前回プレイヤーを発見した時の動作モード
	float_t			timer_;			///< 汎用タイマー

	VectorArray		patrol_point_list_;	///< 巡回ポイントの一覧
	uint_t			current_patrol_point_index_ = 0;

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }
	void on_collide_with( Player* );
	void on_collide_with( Stone* );

protected:
	bool calculate_target_visible() const;
	bool calculate_target_lost() const;

	bool calculate_collide_object_to_switch_off( const GameObject* );

	void update_patrol();

public:
	explicit Robot( graphics::GraphicsManager* graphics_manager );	///< テクスチャを得る先（作り手が渡す。以前は core::Service から取っていた）
	~Robot() { }

	void set_player( const Player* p ) { player_ = p; }

	void restart() override;

	/// 更新
	void update() override;

	void action( const string_t& ) override;

	Mode get_mode() const { return mode_; };
	
	float get_collision_width() const override { return 1.f; }
	float get_collision_height() const override { return 2.f; }
	float get_collision_depth() const  override { return 0.5f; }

	void shutdown();
	void start_floating();

	void add_patrol_point( const Vector& point );

}; // class Robot

} // namespace blue_sky
