#pragma once

#include <Scene/SceneRequest.h>

#include <functional>

namespace core::sound { class SoundManager; }
namespace core::graphics { class DebugDraw; }

namespace blue_sky
{

class ScriptManager;
class ObjectFactory;
class Fader;

namespace graphics { class GraphicsManager; }

/**
 * ゲーム全体で使える Lua の関数を登録する
 *
 * 以前は GameMain::setup_script_command だった（設計上の問題 D2。doc/GameMainの分割.md）。関数の名前と振る舞いは変えていない。
 * 登録した関数は、ここで受け取った窓口を直接呼ぶ（ScriptBindings 自体は登録の後に持ち続けなくてよい）。
 * テストでは、使う窓口だけを渡せばよい（呼ばれない窓口は nullptr でよい）
 */
class ScriptBindings
{
public:
	/// 登録した関数が使う窓口（GameMain の持ち物）
	struct Targets
	{
		graphics::GraphicsManager*							graphics = nullptr;
		core::sound::SoundManager*							sound = nullptr;
		ObjectFactory*										objects = nullptr;
		core::graphics::DebugDraw*							debug_draw = nullptr;		///< デバッグ表示（物理のデバッグ表示の種類を決める）
		Fader*												fader = nullptr;			///< 画面のフェード
		std::function< void( const SceneRequest& ) >		request_scene;		///< シーンの切り替えを要求する（実行はフレームの区切り）
	};

	/// 関数を登録する
	static void bind( ScriptManager& script, const Targets& targets );
};

} // namespace blue_sky
