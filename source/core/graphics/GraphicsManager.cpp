#include "GraphicsManager.h"
#include <core/graphics/RenderTarget.h>

namespace core::graphics
{

void GraphicsManager::set_render_target( RenderTarget* render_target )
{
	render_target->activate();
}

} // namespace core::graphics
