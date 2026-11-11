#pragma once

#include <blue_sky/stage/StageData.h>
#include <blue_sky/GameObjectRef.h>

#include <functional>
#include <optional>
#include <vector>

namespace core::sound { class SoundManager; }
namespace core::physics { class PhysicsManager; }

namespace blue_sky
{

class ObjectFactory;
class Camera;

namespace graphics
{
	class RenderSettings;
}

namespace stage
{

/**
 * 読んだステージ（StageData）をゲームに当てる
 *
 * 当てる順 : BGM → 当たりの形 → 地面 → 物体（書いた順。名前とイベントも）→ プレイヤー → ゴール。
 * 動きを行うのはシーン（GamePlayScene）で、ここは動きをシーンの呼び出しに変える関数（make_call）を受け取る
 */
class StageBuilder
{
public:
	using Call		= std::function< void() >;
	using MakeCall	= std::function< Call( const Action& ) >;

	/// 当てる先（どれも作り手 = シーンが渡す。nullptr の物は使わない）
	struct Context
	{
		ObjectFactory*					object_factory = nullptr;
		core::physics::PhysicsManager*	physics = nullptr;			///< 当たりの形を加える世界
		graphics::RenderSettings*		render_settings = nullptr;	///< 地面を設定する先（描画の設定の窓口）
		core::sound::SoundManager*		sound_manager = nullptr;	///< 読み込むときの BGM
		std::optional< core::ecs::EntityId >	player;					///< プレイヤー（ECS だけの物体。ObjectFactory::create_player で作った物）
		Camera*							camera = nullptr;			///< プレイヤーのカメラの向き
		GameObjectRef					goal;
		string_t						stage_dir;					///< 当たりの形の .obj を探す所（"media/stage/" など）
		MakeCall						make_call;					///< 動きを呼び出しにする（シーンの run_action を呼ぶ）
		std::function< void( float_t ) >	set_bpm;				///< 読み込むときの BGM の BPM
	};

	/// 当てた結果（シーンが持つもの）
	struct Result
	{
		std::vector< Call >		start_calls;		///< 始めるたび（やり直すたび）に呼ぶ（書いた順）
		bool					bgm_loop = true;	///< BGM を繰り返すか
	};

	static Result build( const StageData& data, const Context& context );
};

} // namespace stage

} // namespace blue_sky
