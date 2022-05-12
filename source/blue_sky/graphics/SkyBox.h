#pragma once

#include <blue_sky/graphics/Model.h>

namespace blue_sky::graphics
{

/**
 * SkyBox
 *
 */
class SkyBox : public Model
{
public:

private:
	const char* name_;

protected:
	Shader* create_shader() const override;
	string_t get_texture_file_path_by_texture_name( const char* ) const override;

public:
	explicit SkyBox( const char* );
	~SkyBox() override;

}; // class SkyBox

} // namespace blue_sky::graphics
