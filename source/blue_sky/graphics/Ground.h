#pragma once

#include <blue_sky/graphics/Model.h>

namespace blue_sky::graphics
{

/**
 * 地面
 *
 */
class Ground : public Model
{
public:

protected:
	Shader* create_shader() const override;

public:

}; // class Ground

} // namespace blue_sky::graphics
