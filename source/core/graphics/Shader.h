#pragma once

#include <core/type.h>
#include <vector>

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

	/**
	 * パラメータの型
	 */
	enum class ParameterType
	{
		INT,
		FLOAT,
		VECTOR,
		COLOR,
	};

	/// 各パラメータの情報
	struct ParameterInfo
	{
		ParameterType	type;		// 型
		std::string		name;		// 名前
		float_t			min = 0.f;	// 最小値
		float_t			max = 0.f;	// 最大値
		std::size_t		offset = 0;	// メモリアドレスのオフセット ( 通常 0 で良い )
	};

	/// 全パラメーターの情報
	using ParameterInfoList = std::vector< ParameterInfo >;

public:
	Shader() { }
	virtual ~Shader() { }

	virtual Shader* clone() const = 0;

	virtual const char_t* get_name() const { return typeid( *this ).name(); }

	virtual const ParameterInfoList* get_parameter_info_list() const = 0;

	virtual int_t get_int( const char_t* ) const = 0;
	virtual int_t get_int_at( uint_t ) const = 0;
	
	virtual void set_int( const char_t*, int_t ) = 0;
	virtual void set_int_at( uint_t, int_t ) = 0;

	virtual float_t get_float( const char_t* ) const = 0;
	virtual float_t get_float_at( uint_t ) const = 0;
	
	virtual void set_float( const char_t*, float_t ) = 0;
	virtual void set_float_at( uint_t, float_t ) = 0;

	virtual Vector get_vector( const char_t* ) const = 0;
	virtual Vector get_vector_at( uint_t ) const = 0;

	virtual void set_vector( const char_t*, const Vector& ) = 0;
	virtual void set_vector_at( uint_t, const Vector& ) = 0;

	virtual Color get_color( const char_t* ) const = 0;
	virtual Color get_color_at( uint_t ) const = 0;

	virtual void set_color( const char_t*, const Color& ) = 0;
	virtual void set_color_at( uint_t, const Color& ) = 0;

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

	virtual void render_parameter_gui();

}; // Shader

} // namespace core::graphics
