#include "SkyBox.h"
#include <blue_sky/graphics/shader/SkyBoxShader.h>
#include <blue_sky/GameMain.h>

namespace blue_sky::graphics
{

SkyBox::SkyBox( const char* name )
	: name_( name )
{
	
}

SkyBox::~SkyBox()
{

}

SkyBox::Shader* SkyBox::create_shader() const
{
	return GameMain::get_instance()->get_graphics_manager()->create_shader< shader::SkyBoxShader >();
}

string_t SkyBox::get_texture_file_path_by_texture_name( const char* texture_name ) const
{
	return std::string( "media/model/" ) + name_ + "-" + texture_name + ".png";
}

} // namespace blue_sky::graphics
