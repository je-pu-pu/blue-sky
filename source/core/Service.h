#pragma once

#include <common/Singleton.h>

namespace core
{

class TimeManager;

namespace graphics { class GraphicsManager; }
namespace physics { class PhysicsManager; }
namespace sound { class SoundManager; }
namespace input { class InputManager; }

/**
 * サービスロケーター（core 層用）
 *
 * ゲーム全体で共有されるマネージャーへのアクセスを提供する。
 *
 * サービスの探し方の決まり（設計上の問題 B28）:
 * - core 層（core::ecs のシステム・core::graphics など）は、ここから core の型で得る。core 層は blue_sky 層（GameMain）を知らない
 * - blue_sky 層は GameMain（組み立ての根）から得る。Scene の get_*_manager() は GameMain への近道
 * - 新しく書くロジックは、できるだけ依存を引数・コンストラクタで受け取る（例 : OptionsController）。テストで差し替えられる
 * - テストでは Service::Scope で一時的に差し替える
 *
 * 注意 : core::get_graphics_manager() は core::graphics::GraphicsManager を、
 *        Scene::get_graphics_manager() は blue_sky::graphics::GraphicsManager を返す（名前が同じで型が違う）。
 */
class Service : public common::Singleton<Service>
{
private:
	graphics::GraphicsManager* graphics_manager_ = nullptr;
	physics::PhysicsManager* physics_manager_ = nullptr;
	sound::SoundManager* sound_manager_ = nullptr;
	input::InputManager* input_manager_ = nullptr;
	TimeManager* time_manager_ = nullptr;

public:
	// Setters (GameMain から呼び出す)
	void set_graphics_manager(graphics::GraphicsManager* m) { graphics_manager_ = m; }
	void set_physics_manager(physics::PhysicsManager* m) { physics_manager_ = m; }
	void set_sound_manager(sound::SoundManager* m) { sound_manager_ = m; }
	void set_input_manager(input::InputManager* m) { input_manager_ = m; }
	void set_time_manager(TimeManager* m) { time_manager_ = m; }

	// Getters (System 等から呼び出す)
	graphics::GraphicsManager* get_graphics_manager() const { return graphics_manager_; }
	physics::PhysicsManager* get_physics_manager() const { return physics_manager_; }
	sound::SoundManager* get_sound_manager() const { return sound_manager_; }
	input::InputManager* get_input_manager() const { return input_manager_; }
	TimeManager* get_time_manager() const { return time_manager_; }

	/**
	 * サービスを一時的に差し替える（テスト用）。破棄すると元に戻す
	 *
	 * 例 : Service::Scope scope; scope.service().set_physics_manager( & fake_physics );
	 */
	class Scope
	{
	private:
		graphics::GraphicsManager* graphics_manager_;
		physics::PhysicsManager* physics_manager_;
		sound::SoundManager* sound_manager_;
		input::InputManager* input_manager_;
		TimeManager* time_manager_;

	public:
		Scope()
		{
			const Service* s = Service::get_instance();
			graphics_manager_ = s->graphics_manager_;
			physics_manager_ = s->physics_manager_;
			sound_manager_ = s->sound_manager_;
			input_manager_ = s->input_manager_;
			time_manager_ = s->time_manager_;
		}

		~Scope()
		{
			Service* s = Service::get_instance();
			s->graphics_manager_ = graphics_manager_;
			s->physics_manager_ = physics_manager_;
			s->sound_manager_ = sound_manager_;
			s->input_manager_ = input_manager_;
			s->time_manager_ = time_manager_;
		}

		Scope( const Scope& ) = delete;
		Scope& operator = ( const Scope& ) = delete;

		Service& service() { return * Service::get_instance(); }
	};
};

// 短縮アクセス関数
inline graphics::GraphicsManager* get_graphics_manager()
{
	return Service::get_instance()->get_graphics_manager();
}

inline physics::PhysicsManager* get_physics_manager()
{
	return Service::get_instance()->get_physics_manager();
}

inline sound::SoundManager* get_sound_manager()
{
	return Service::get_instance()->get_sound_manager();
}

inline input::InputManager* get_input_manager()
{
	return Service::get_instance()->get_input_manager();
}

inline TimeManager* get_time_manager()
{
	return Service::get_instance()->get_time_manager();
}

} // namespace core
