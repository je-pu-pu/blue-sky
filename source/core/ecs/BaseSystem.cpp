#include "System.h"

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/GameMain.h>

namespace core::ecs
{

BaseSystem::GraphicsManager* BaseSystem::get_graphics_manager()
{
	return blue_sky::GameMain::get_instance()->get_graphics_manager();
}

} // namespace core::ecs
