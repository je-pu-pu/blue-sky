#include "Ground.h"
#include <blue_sky/graphics/shader/GroundShader.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <core/graphics/GraphicsManager.h>
#include <core/Service.h>

namespace blue_sky::graphics
{

Ground::Shader* Ground::create_shader() const
{
	return graphics::get_graphics_manager()->create_shader< shader::GroundShader >();
}

} // namespace blue_sky::graphics
