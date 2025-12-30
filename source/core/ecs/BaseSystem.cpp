#include "System.h"

#include <core/Service.h>
#include <blue_sky/GameMain.h> /// @todo get_current_scene() のために残している。Scene を Service に登録するか、別の方法で解決する

namespace core::ecs
{

BaseSystem::GraphicsManager* BaseSystem::get_graphics_manager()
{
	return core::get_graphics_manager();
}

const BaseSystem::Scene* BaseSystem::get_current_scene() const
{
	return blue_sky::GameMain::get_instance()->get_current_scene();
}

} // namespace core::ecs
