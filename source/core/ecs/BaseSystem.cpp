#include "System.h"

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/GameMain.h>

namespace core::ecs
{

BaseSystem::GraphicsManager* BaseSystem::get_graphics_manager()
{
	return blue_sky::GameMain::get_instance()->get_graphics_manager();
}

const BaseSystem::Scene* BaseSystem::get_current_scene() const
{
	return blue_sky::GameMain::get_instance()->get_current_scene();
}

} // namespace core::ecs
