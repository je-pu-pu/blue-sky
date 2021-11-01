#pragma once

#include "BaseShader.h"
#include <core/graphics/Direct3D11/ConstantBuffer.h> /// @todo Direct3D11 を直接参照しない
#include <common/enum_set.h>
#include <vector>
#include <functional>

namespace blue_sky::graphics::shader
{

/**
 * シェーダーの共通機能
 * 
 * 以下の情報を保持する
 *		入力レイアウト
 *		テクニック
 *		シェーダー固有の名前付きパラメータ
 */
template< typename ShaderType, int Slot >
class ShaderMixin
{
public:
	using InputLayout		= core::graphics::InputLayout;
	using EffectTechnique	= core::graphics::EffectTechnique;

	/**
	 * シェーダーステージ
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
	 * パラメータの型
	 */
	enum class ParameterType
	{
		SCALAR,
		VECTOR,
		COLOR,
	};

	/// シェーダーステージの集合
	// using ShaderStageSet = common::enum_set< ShaderStage, static_cast< size_t >( ShaderStage::Max ) >;
	using ShaderStageSet = common::enum_set< ShaderStage, 32 >;

	/// 各パラメータの情報
	struct ParameterInfo
	{
		ParameterType	type;	// 型
		std::string		name;	// 名前
		std::size_t		offset;	// メモリアドレスのオフセット ( 通常 0 で良い )
	};

protected:
	const InputLayout* input_layout_;
	const EffectTechnique* effect_technique_;

	std::map< string_t, int > scalar_index_map_;
	std::map< string_t, int > vector_index_map_;
	std::map< string_t, int > color_index_map_;

	std::vector< float_t* > scalar_address_list_;
	std::vector< Vector* > vector_address_list_;
	std::vector< Color* > color_address_list_;

	uint8_t* buffer_ = nullptr;
	ConstantBuffer< Slot > constant_buffer_;

	std::vector< BaseShader::Texture* > textures_;

	ShaderStageSet shader_stage_set_;

	// void setup_color_parameter( int, const char_t*, ShaderStage );

	/// シェーダーステージ => そのシェーダーステージへ定数バッファをバインドする関数のマップ
	static inline const std::vector< std::function< void ( const ConstantBuffer< Slot >& ) > > bind_function_list_ = {
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_vs(); } },
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_hs(); } },
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_ds(); } },
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_cs(); } },
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_gs(); } },
		{ [] ( const ConstantBuffer< Slot >& constant_buffer ) { constant_buffer.bind_to_ps(); } },
	};

	/**
	 * シェーダーに対して設定できる固有のパラメータをセットアップする
	 * 
	 * @param params パラメータ情報の一覧
	 * @return パラメータの合計サイズ ( バイト数 )
	 */
	size_t setup_parameters( std::initializer_list< ParameterInfo > params )
	{
		std::size_t size = 0;

		for ( auto p : params )
		{
			size += p.offset;

			switch ( p.type )
			{
				case ParameterType::SCALAR: size += sizeof( float_t ); break;
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
				case ParameterType::SCALAR:
					scalar_address_list_.resize( scalar_index_map_.size() + 1 );
					scalar_address_list_[ scalar_index_map_.size() ] = reinterpret_cast< float_t* >( pointer );
					scalar_index_map_[ p.name ] = scalar_index_map_.size();
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
	 * パラメータ名を指定して色を取得する
	 * 
	 * @param name パラメータ名
	 * @return 色
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
	 * パラメータ名を指定して色を設定する
	 * 
	 * @param name パラメータ名
	 * @param color 設定する色
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

	/**
	 * GPU メモリを更新する
	 */
	void update() const
	{
		constant_buffer_.update( buffer_ );
	}

	/**
	 * 定数バッファを必要なシェーダーステージで使えるようにバインドする
	 */
	void bind() const
	{
		/// @todo 必要なシェーダーにのみバインドする
		// constant_buffer_.bind_to_all();

		/*
		for ( int n = 0; n < static_cast< int >( ShaderStage::Max ); n++ )
		{
			if ( shader_stage_set_.test( n ) )
			{
				shader_stage_bind_function_map_[ n ]();
			}
		}
		*/

		for ( auto n = 0; n < bind_function_list_.size(); n++ )
		{
			if ( shader_stage_set_.test( static_cast< ShaderStage >( n ) ) )
			{
				bind_function_list_[ n ]( constant_buffer_ );
			}
		}
	}
};

} // namespace blue_sky::graphics::shader
