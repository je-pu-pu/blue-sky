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
 * サービスロケーター
 *
 * ゲーム全体で共有されるマネージャーへのアクセスを提供する
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
