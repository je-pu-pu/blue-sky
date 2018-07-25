#include "BaseFileLoader.h"

#include <GameMain.h>
#include <blue_sky/graphics/shader/FlatShader.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Mesh.h>
#include <core/type.h>

#include <game/Shader.h>

namespace blue_sky::graphics
{

BaseFileLoader::Mesh* BaseFileLoader::create_mesh() const
{
	return GameMain::get_instance()->get_graphics_manager()->create_named_mesh( get_name().c_str() );
}

BaseFileLoader::Shader* BaseFileLoader::create_shader() const
{
	/// @todo ‰e‚ª—LŒø‚Èê‡‚É‘Î‰ž‚·‚é
	if ( get_model()->is_skin_mesh() )
	{
		return GameMain::get_instance()->get_graphics_manager()->create_shader< shader::FlatShader >( "skin", "flat_skin" );
	}
	else
	{
		return GameMain::get_instance()->get_graphics_manager()->create_shader< shader::FlatShader >( "main", "flat" );
	}
}

BaseFileLoader::SkinningAnimationSet* BaseFileLoader::create_skinning_animation_set() const
{
	return GameMain::get_instance()->get_graphics_manager()->create_skinning_animation_set();
}

} // namespace blue_sky::graphics
