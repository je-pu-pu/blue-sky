#include "Ground.h"
#include <blue_sky/graphics/shader/GroundShader.h>
#include <blue_sky/GameMain.h>

namespace blue_sky::graphics
{

Ground::Shader* Ground::create_shader() const
{
	return GameMain::get_instance()->get_graphics_manager()->create_shader< shader::GroundShader >();
}

} // namespace blue_sky::graphics
