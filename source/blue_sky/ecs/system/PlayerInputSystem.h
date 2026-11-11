#pragma once

#include <core/ecs/System.h>
#include <blue_sky/ecs/component/PlayerInputComponent.h>
#include <blue_sky/ecs/component/PlayerBehaviorComponent.h>

namespace blue_sky::ecs
{

class PlayerBehaviorSystem;

/**
 * プレイヤーへの入力（PlayerInputComponent）から、プレイヤーを操作する（以前の GamePlayScene::update_main のプレイヤーの所）
 *
 * 入力は、シーンが毎フレーム Input から書く（開発用の確かめは決まった入力の並びを、テストは偽の入力を書く）。
 * 操作は PlayerBehaviorSystem の関数を、以前の update_main と同じ順・同じ条件で呼ぶ。
 * カメラ（シーンの Camera。Entity ではない）は、操作の前に View で渡してもらい、操作の後にシーンが Result に従って動かす
 * （以前は update_main がプレイヤーの操作の途中でカメラを動かしていたが、カメラの動きはプレイヤーの操作の結果を変えないので、後にまとめても同じ）。
 *
 * EntityManager::update() では動かさない（本編のシーンが、入力を書いた直後に update_player を呼ぶ）
 */
class PlayerInputSystem : public core::ecs::System< PlayerInputComponent, PlayerBehaviorComponent >
{
public:
	/// 操作に使うカメラの様子（シーンが渡す）
	struct View
	{
		float_t			camera_rotate_x = 0.f;					///< カメラの上下の向き（度。マウスの縦の移動を足した後。正なら見下ろす）
		core::Vector	camera_front = core::Vector::Forward;	///< カメラの前（ロケット・石を投げる向き）
		float_t			rotation_speed_rate = 1.f;				///< 視野の広さの割合（ズーム中はマウスで向きを変える速さが遅くなる）
		float_t			mouse_rotation_speed = 90.f;			///< マウスの移動量 1 あたりの向きの変化（度）
		bool			is_mouse_look = true;					///< マウスで上下を見るか（ヘッドマウントディスプレイが無い）
	};

	/// 操作の結果、シーンがカメラ・画面に行うこと
	struct Result
	{
		bool			is_dead = false;			///< プレイヤーが死んでいた（操作しなかった）
		bool			restart_requested = false;	///< 死んでいる間に A が押された（シーンがやり直す）
		bool			scope_started = false;		///< 望遠鏡をのぞき始めた（カメラの視野を狭める）
		int				scope_wheel = 0;			///< 望遠鏡をのぞいている間のホイール（カメラの視野を変える）
		bool			reset_fov = false;			///< 望遠鏡をのぞいていない（カメラの視野を戻す）
	};

private:
	PlayerBehaviorSystem* find_behavior() const;

public:
	PlayerInputSystem() = default;

	void update() override { }		///< 何もしない（上の説明のとおり）

	/// 入力の部品の入力で、プレイヤーを操作する（プレイヤーでなければ何もしない）
	Result update_player( core::ecs::EntityId id, const View& view );

	/// 入力の部品（プレイヤーでなければ nullptr）
	PlayerInputComponent* find_input( core::ecs::EntityId id ) const;
};

} // namespace blue_sky::ecs
