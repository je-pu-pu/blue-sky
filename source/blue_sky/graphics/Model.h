#pragma once

#include <game/Model.h>
#include <vector>

namespace blue_sky::graphics
{
/**
 * blue_sky ƒ‚ƒfƒ‹ƒNƒ‰ƒX
 *
 */
class Model : public game::Model
{
public:
	typedef game::Mesh Mesh;
	typedef game::Shader Shader;
	typedef std::vector< Shader* > ShaderList;

private:
	Mesh* mesh_;
	ShaderList shader_list_;

public:
	Model() { }
	virtual ~Model() { }

	Mesh* get_mesh() { return mesh_; };
	void set_mesh( Mesh* m ) { mesh_ = m; }

	Shader* get_shader_at( uint_t n ) { return shader_list_[ n ]; }
	void set_shader_at( uint_t n , Shader* s ) { shader_list_[ n ] = s; }

	uint_t get_shader_count() const { return shader_list_.size(); }
};

} // namespace game
