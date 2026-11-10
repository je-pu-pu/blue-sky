#pragma once

#include <core/ecs/Component.h>
#include <core/graphics/Model.h>
#include <core/graphics/ConstantBuffer.h>
#include <core/type.h>

#include <memory>

namespace blue_sky::ecs
{

/**
 * ゲームの物体を描くためのデータ（モデル・定数バッファ・前フレームの world・描くかどうか）
 *
 * 以前は ActiveObject が持っていた（Model* の生ポインタ・定数バッファ・前フレームの world）。
 * ObjectRenderDataSystem が定数を書き、GraphicsManager が描く対象として回す。
 * 位置は同じ Entity の core::ecs::TransformComponent を使う。
 *
 * 橋渡しの間（段階 1〜3）は、位置と描くかどうかの正は ActiveObject にあり、
 * ActiveObjectManager::sync_render_components が描く前に毎フレームここへ写す（一方向）。
 */
struct ObjectRenderComponent : public core::ecs::Component
{
	core::graphics::ModelHandle model;		///< モデル（GraphicsManager::resolve_model で引く。解放されたら描かない。差し替えは番号を変えずに中身が替わる）

	std::unique_ptr< const core::graphics::ConstantBuffer > object_constant_buffer;	///< world・前フレームの world・線の色（毎フレーム書き換える）
	std::unique_ptr< const core::graphics::ConstantBuffer > bone_constant_buffer;	///< ボーン行列（アニメーションするモデルだけ。無ければ nullptr）

	core::Matrix	prev_world;					///< 前フレームの world 行列（モーションベクトル用）
	bool			prev_world_valid = false;	///< prev_world が有効か（初回は今の world を前フレームの world にして速度 0 にする）

	bool			is_visible = true;			///< 定数を書くか（ActiveObject::is_visible() を写す）
	bool			is_mesh_visible = true;		///< メッシュを描くか（ActiveObject::is_mesh_visible() を写す。派生が条件を足すので、写した後の値を使う）
	bool			is_line_visible = true;		///< 線を描くか（同上）
};

} // namespace blue_sky::ecs
