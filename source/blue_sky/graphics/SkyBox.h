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
	virtual Shader* create_shader() const;
	virtual string_t get_texture_file_path_by_texture_name( const char* ) const;

public:
	explicit SkyBox( const char* );
	~SkyBox();

}; // class SkyBox

} // namespace blue_sky::graphics
