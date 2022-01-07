#pragma once

#include "ActiveObject.h"
#include <array>

namespace blue_sky
{

/**
 * パーティクルシステム
 *
 * @todo コンポーネントにする
 */
class ParticleSystem : public ActiveObject
{
public:
	struct Vertex
	{
		Vector3 Position;
	};

	ConstantBuffer< 5 > constant_buffer_;

protected:
	std::array< Vertex, 1000 > particle_list_;

public:
	ParticleSystem();
	~ParticleSystem();


	float_t get_collision_width() const override { return 0.f; };
	float_t get_collision_height() const override { return 0.f; };
	float_t get_collision_depth() const override { return 0.f; };

	void update() override;

	void update_render_data() const override;
	void bind_render_data() const override;

}; // class ParticleSystem

} // namespace blue_sky
