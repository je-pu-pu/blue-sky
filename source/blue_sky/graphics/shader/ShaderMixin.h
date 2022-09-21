#pragma once

#include "BaseShader.h"
#include <core/graphics/Direct3D11/ConstantBuffer.h> /// @todo Direct3D11 �𒼐ڎQ�Ƃ��Ȃ�
#include <common/enum_set.h>
#include <vector>
#include <functional>

namespace blue_sky::graphics::shader
{

/**
 * �V�F�[�_�[�̋��ʋ@�\
 * 
 * �ȉ��̏���ێ�����
 *		���̓��C�A�E�g
 *		�e�N�j�b�N
 *		�V�F�[�_�[�ŗL�̖��O�t���p�����[�^
 */
template< typename ShaderType, int Slot >
class ShaderMixin
{
public:
	using InputLayout		= core::graphics::InputLayout;
	using EffectTechnique	= core::graphics::EffectTechnique;

	/**
	 * �V�F�[�_�[�X�e�[�W
	 */
	enum class ShaderStage
	{
		VS,
		HS,
		DS,
		CS,
		GS,
		PS,

		Max,
	};

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

	/// �V�F�[�_�[�X�e�[�W�̏W��
	// using ShaderStageSet = common::enum_set< ShaderStage, static_cast< size_t >( ShaderStage::Max ) >;
	using ShaderStageSet = common::enum_set< ShaderStage, 32 >;

	/// �e�p�����[�^�̏��
	struct ParameterInfo
	{
		ParameterType	type;	// �^
		std::string		name;	// ���O
		std::size_t		offset;	// �������A�h���X�̃I�t�Z�b�g ( �ʏ� 0 �ŗǂ� )
	};

protected:
	const InputLayout* input_layout_;
	const EffectTechnique* effect_technique_;

	std::map< string_t, int > int_index_map_;
	std::map< string_t, int > float_index_map_;
	std::map< string_t, int > vector_index_map_;
	std::map< string_t, int > color_index_map_;

	std::vector< int_t* > int_address_list_;
	std::vector< float_t* > float_address_list_;
	std::vector< Vector* > vector_address_list_;
	std::vector< Color* > color_address_list_;

	uint8_t* buffer_ = nullptr;
	ConstantBuffer< Slot > constant_buffer_;

	std::vector< BaseShader::Texture* > textures_;

	ShaderStageSet shader_stage_set_;

	/// �V�F�[�_�[�X�e�[�W => ���̃V�F�[�_�[�X�e�[�W�֒萔�o�b�t�@���o�C���h����֐��̃}�b�v
	static inline const std::vector< std::function< void ( const ConstantBuffer< Slot >& ) > > bind_function_list_ = {
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_vs(); } },
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_hs(); } },
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_ds(); } },
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_cs(); } },
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_gs(); } },
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_ps(); } },
	};

	/**
	 * �V�F�[�_�[�ɑ΂��Đݒ�ł���ŗL�̃p�����[�^���Z�b�g�A�b�v����
	 * 
	 * @param params �p�����[�^���̈ꗗ
	 * @return �p�����[�^�̍��v�T�C�Y ( �o�C�g�� )
	 */
	size_t setup_parameters( std::initializer_list< ParameterInfo > params )
	{
		std::size_t size = 0;

		for ( auto p : params )
		{
			size += p.offset;

			switch ( p.type )
			{
				case ParameterType::INT:	size += sizeof( int_t ); break;
				case ParameterType::FLOAT:	size += sizeof( float_t ); break;
				case ParameterType::VECTOR: size += sizeof( Vector ); break;
				case ParameterType::COLOR:  size += sizeof( Color ); break;
			}
		}

		delete [] buffer_;
		buffer_ = new uint8_t[ size ];
		std::fill_n( buffer_, size, 0 );

		uint8_t* pointer = buffer_;

		for ( auto p : params )
		{
			pointer += p.offset;

			switch ( p.type )
			{
				case ParameterType::INT:
					int_address_list_.resize( int_index_map_.size() + 1 );
					int_address_list_[ int_index_map_.size() ] = reinterpret_cast< int_t* >( pointer );
					int_index_map_[ p.name ] = int_index_map_.size();
					pointer += sizeof( int_t );
					break;
				case ParameterType::FLOAT:
					float_address_list_.resize( float_index_map_.size() + 1 );
					float_address_list_[ float_index_map_.size() ] = reinterpret_cast< float_t* >( pointer );
					float_index_map_[ p.name ] = float_index_map_.size();
					pointer += sizeof( float_t );
					break;
				case ParameterType::VECTOR:
					vector_address_list_.resize( vector_index_map_.size() + 1 );
					vector_address_list_[ vector_index_map_.size() ] = reinterpret_cast< Vector* >( pointer );
					vector_index_map_[ p.name ] = vector_index_map_.size();
					pointer += sizeof( Vector );
					break;
				case ParameterType::COLOR:
					color_address_list_.resize( color_index_map_.size() + 1 );
					color_address_list_[ color_index_map_.size() ] = reinterpret_cast< Color* >( pointer );
					color_index_map_[ p.name ] = color_index_map_.size();
					pointer += sizeof( Color );
					break;
			}
		}

		return size;
	}

public:
	ShaderMixin( const char_t* input_layout_name, const char_t* effect_technique_name, std::initializer_list< ParameterInfo > params, ShaderStageSet sss )
		: input_layout_( GameMain::get_instance()->get_graphics_manager()->get_input_layout( input_layout_name ) )
		, effect_technique_( GameMain::get_instance()->get_graphics_manager()->get_effect_technique( effect_technique_name ) )
		, constant_buffer_( setup_parameters( params ) )
		, shader_stage_set_( sss )
	{

	}

	~ShaderMixin()
	{
		delete [] buffer_;
	}

	/**
	 * �p�����[�^�����w�肵�Đ������擾����
	 * 
	 * @param name �p�����[�^��
	 * @return ����
	 */
	int_t get_int( const string_t& name ) const
	{
		auto i = int_index_map.find( name );

		if ( i == int_index_map_.end() )
		{
			return 0;
		}

		return * int_address_list_[ i->second ];
	}

	/**
	 * �p�����[�^�����w�肵�Đ�����ݒ肷��
	 * 
	 * @param name �p�����[�^��
	 * @param value �ݒ肷�鐮��
	 */
	void set_int( const string_t& name, int_t value )
	{
		auto i = int_index_map_.find( name );

		if ( i == int_index_map_.end() )
		{
			return;
		}

		* int_address_list_[ i->second ] = value;
	}

	/**
	 * �p�����[�^�����w�肵�Ď������擾����
	 * 
	 * @param name �p�����[�^��
	 * @return ����
	 */
	float_t get_float( const string_t& name ) const
	{
		auto i = float_index_map.find( name );

		if ( i == float_index_map_.end() )
		{
			return 0.f;
		}

		return * float_address_list_[ i->second ];
	}

	/**
	 * �p�����[�^�����w�肵�Ď�����ݒ肷��
	 * 
	 * @param name �p�����[�^��
	 * @param value �ݒ肷�����
	 */
	void set_float( const string_t& name, float_t value )
	{
		auto i = float_index_map_.find( name );

		if ( i == float_index_map_.end() )
		{
			return;
		}

		* float_address_list_[ i->second ] = value;
	}

	/**
	 * �p�����[�^�����w�肵�ĐF���擾����
	 * 
	 * @param name �p�����[�^��
	 * @return �F
	 */
	Color get_color( const string_t& name ) const
	{
		auto i = color_index_map_.find( name );

		if ( i == color_index_map_.end() )
		{
			return Color::Black;
		}

		return *color_address_list_[ i->second ];
	}

	/**
	 * �p�����[�^�����w�肵�ĐF��ݒ肷��
	 * 
	 * @param name �p�����[�^��
	 * @param color �ݒ肷��F
	 */
	void set_color( const string_t& name, const Color& color )
	{
		auto i = color_index_map_.find( name );

		if ( i == color_index_map_.end() )
		{
			return;
		}

		*color_address_list_[ i->second ] = color;
	}

	BaseShader::Texture* get_texture_at( uint_t n )
	{
		return  n < textures_.size() ? textures_[ n ] : nullptr;
	}
	
	const BaseShader::Texture* get_texture_at( uint_t n ) const
	{
		return  n < textures_.size() ? textures_[ n ] : nullptr;
	}

	void set_texture_at( uint_t n , BaseShader::Texture* t )
	{
		if ( n >= textures_.size() )
		{
			textures_.resize( n + 1, nullptr );
		}

		textures_[ n ] = t;
	}

	/**
	 * GPU ���������X�V����
	 */
	void update() const
	{
		constant_buffer_.update( buffer_ );
	}

	/**
	 * �萔�o�b�t�@�ƃe�N�X�`����K�v�ȃV�F�[�_�[�X�e�[�W�Ŏg����悤�Ƀo�C���h����
	 */
	void bind() const
	{
		// �K�v�ȃV�F�[�_�[�ɂ̂݃o�C���h����
		for ( size_t n = 0; n < bind_function_list_.size(); n++ )
		{
			if ( shader_stage_set_.test( static_cast< ShaderStage >( n ) ) )
			{
				bind_function_list_[ n ]( constant_buffer_ );
			}
		}

		/// @tood �K�v�ȃV�F�[�_�[�ɂ̂݃o�C���h����
		for ( auto n = 0; n < textures_.size(); n++ )
		{
			textures_[ n ]->bind_to_ps( n );
		}
	}
};

} // namespace blue_sky::graphics::shader
