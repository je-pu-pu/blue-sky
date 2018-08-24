#pragma once

#include <blue_sky/type.h>

namespace game
{
	class VertexGroup;
}

namespace blue_sky
{
	namespace graphics
	{
		class Model;
		class Mesh;
	}

/**
 * äXÇÃé©ìÆê∂ê¨
 *
 */
class CityGenerator
{
public:
	using Model			= graphics::Model;
	using Mesh			= graphics::Mesh;
	using VertexGroup	= game::VertexGroup;

private:
	Model* model_;
	Vector control_point_;
	// float_t direction_ = 0.f;

protected:
	void extend_road( const Vector&, const Vector&, const Vector&, float_t, int_t, int_t );

public:
	CityGenerator();

	void step();

	Model* get_model() { return model_; }
};

} // namespace blue_sky
