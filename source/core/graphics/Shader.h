#pragma once

#include <core/type.h>
#include <vector>

namespace core::graphics
{
	class Model;
	class Mesh;
	class Texture;

/**
 * ���ۃV�F�[�_�[�N���X
 *
 * @todo scalar, vector, color, texture �ɖ��O��t���Đݒ�A���O���w�肵�Ď擾�ł���悤�ɂ���
 */
class Shader
{
public:
	using Vector = core::Vector;
	using Color = core::Color;
	using Texture = Texture;

	/**
	 * �p�����[�^�̌^
	 */
	enum class ParameterType
	{
		INT,
		FLOAT,
		VECTOR,
		COLOR,
	};

	/// �e�p�����[�^�̏��
	struct ParameterInfo
	{
		ParameterType	type;		// �^
		std::string		name;		// ���O
		float_t			min = 0.f;	// �ŏ��l
		float_t			max = 0.f;	// �ő�l
		std::size_t		offset = 0;	// �������A�h���X�̃I�t�Z�b�g ( �ʏ� 0 �ŗǂ� )
	};

	/// �S�p�����[�^�[�̏��
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
