#pragma once

#include <type/type.h>

namespace core::graphics
{

class Mesh;
class Shader;

/**
 * 抽象モデルクラス
 *
 * 以下の情報を保持する
 * * Mesh ( どのような形状を持つか？ )
 * * Shader ( どう描画するか？ )
 * 
 */
class Model
{
public:

public:
	Model() { }
	virtual ~Model() { }

	virtual Mesh* get_mesh() = 0;
	virtual const Mesh* get_mesh() const = 0;
	virtual void set_mesh( Mesh* ) = 0;

	virtual Shader* get_shader_at( uint_t ) = 0;
	virtual const Shader* get_shader_at( uint_t ) const = 0;
	virtual void set_shader_at( uint_t, Shader* ) = 0;

	virtual uint_t get_shader_count() const = 0;

	virtual void render() const = 0;
};

} // namespace core::graphics
