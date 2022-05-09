#pragma once

#include <type/type.h>

namespace core::graphics
{
	class Model;
	class Mesh;
	class Texture;

/**
 * 抽象シェーダークラス
 *
 * @todo scalar, vector, color, texture に名前を付けて設定、名前を指定して取得できるようにする
 */
class Shader
{
public:
	using Vector = core::Vector;
	using Color = core::Color;
	using Texture = Texture;

public:
	Shader() { }
	virtual ~Shader() { }

	virtual Shader* clone() const = 0;

	virtual const char_t* get_name() const { return typeid( *this ).name(); }

	virtual float_t get_scalar( const char_t* ) const = 0;
	virtual float_t get_scalar_at( uint_t ) const = 0;
	
	virtual void set_scalar( const char_t*, float_t ) = 0;
	virtual void set_scalar_at( uint_t, float_t ) = 0;

	virtual Vector get_vector( const char_t* ) const = 0;
	virtual Vector get_vector_at( uint_t ) const = 0;

	virtual void set_vector( const char_t*, Vector ) = 0;
	virtual void set_vector_at( uint_t, Vector ) = 0;

	virtual Color get_color( const char_t* ) const = 0;
	virtual Color get_color_at( uint_t ) const = 0;

	virtual void set_color( const char_t*, Color ) = 0;
	virtual void set_color_at( uint_t, Color ) = 0;

	virtual Texture* get_texture( const char_t* ) = 0;
	virtual Texture* get_texture_at( uint_t ) = 0;

	virtual const Texture* get_texture( const char_t* ) const = 0;
	virtual const Texture* get_texture_at( uint_t ) const = 0;

	virtual void set_texture( const char_t*, Texture* ) = 0;
	virtual void set_texture_at( uint_t, Texture* ) = 0;

	virtual void update() const = 0;
	virtual void bind() const = 0;
	virtual void render_model( const Model* ) const = 0;
	virtual void render( const Mesh*, uint_t ) const = 0;

}; // Shader

} // namespace core::graphics
