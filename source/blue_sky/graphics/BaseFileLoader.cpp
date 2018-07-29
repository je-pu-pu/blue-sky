#include "BaseFileLoader.h"

#include <GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Mesh.h>
#include <core/type.h>

#include <game/Shader.h>

#include <boost/filesystem.hpp>

namespace blue_sky::graphics
{

BaseFileLoader::Mesh* BaseFileLoader::create_mesh() const
{
	return GameMain::get_instance()->get_graphics_manager()->create_named_mesh( get_name().c_str() );
}

BaseFileLoader::Shader* BaseFileLoader::create_shader() const
{
	return get_model()->create_shader();
}

BaseFileLoader::SkinningAnimationSet* BaseFileLoader::create_skinning_animation_set() const
{
	return GameMain::get_instance()->get_graphics_manager()->create_skinning_animation_set();
}

BaseFileLoader::Texture* BaseFileLoader::load_texture( const char_t* name ) const
{
	const string_t file_path = get_model()->get_texture_file_path_by_texture_name( name );

	return GameMain::get_instance()->get_graphics_manager()->load_named_texture( name, file_path.c_str() );
}

} // namespace blue_sky::graphics
