#pragma once

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/TextureSlot.h>

#include <core/animation/AnimationPlayer.h>
#include <core/graphics/PrimitiveTopology.h>

#include <core/graphics/Model.h>
#include <core/graphics/Mesh.h>

#include <core/math/Vector.h>

#include <iostream>

namespace blue_sky::graphics::shader
{

/**
 * blue-sky シェーダー基底クラス
 *
 */
class BaseShader : public core::graphics::Shader
{
public:
	using Mesh				= core::graphics::Mesh;

	using PrimitiveTopology	= core::graphics::PrimitiveTopology;
	using InputLayout		= core::graphics::InputLayout;
	using EffectTechnique	= core::graphics::EffectTechnique;
	using ConstantBuffer	= core::graphics::ConstantBuffer;

	/**
	 * 入力レイアウトとテクニック
	 *
	 * GraphicsManager はシェーダーから受け取る（以前は core::Service から取っていた。設計上の問題 B28）
	 */
	class RenderSetting
	{
	private:
		const core::graphics::GraphicsManager* graphics_manager_;
		const InputLayout* input_layout_;
		const string_t effect_technique_name_;
		const EffectTechnique* effect_technique_;

	public:
		RenderSetting( const core::graphics::GraphicsManager* graphics_manager, const char_t* input_layout_name, const char_t* effect_technique_name )
			: graphics_manager_( graphics_manager )
			, input_layout_( graphics_manager->get_input_layout( input_layout_name ) )
			, effect_technique_name_( effect_technique_name )
			, effect_technique_( graphics_manager->get_effect_technique( effect_technique_name ) )
		{
			if ( ! input_layout_ )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( string_t( "input layout \"" ) + input_layout_name +  "\" not found." );
			}
		
			if ( ! effect_technique_ )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( string_t( "effect technique \"" ) + effect_technique_name + "\"not found." );
			}
		}

		void reload()
		{
			effect_technique_ = graphics_manager_->get_effect_technique( effect_technique_name_.c_str() );
		}

		const InputLayout* get_input_layout() const { return input_layout_; }
		const EffectTechnique* get_effect_technique() const { return effect_technique_; }
	};

private:
	GraphicsManager* graphics_manager_;	///< 作り手（GraphicsManager::create_shader）が渡す。コピー（clone）は引き継ぐ。以前は core::Service から取っていた

	mutable bool texture_lost_logged_ = false;	///< 解放されたテクスチャのログを出したか（毎フレーム出さない）

protected:
	using TextureSlot = graphics::TextureSlot;

	GraphicsManager* get_graphics_manager() const { return graphics_manager_; }

	/// テクスチャのスロットを引く（空・解放済みなら nullptr）
	Texture* resolve_texture( const TextureSlot& slot ) const { return slot.get( * graphics_manager_ ); }

	/// テクスチャのスロットをピクセルシェーダーに結ぶ（空・解放済みなら結ばない）
	void bind_texture_to_ps( const TextureSlot& slot, uint_t n ) const
	{
		if ( Texture* t = resolve_texture( slot ) )
		{
			t->bind_to_ps( n );
		}
	}

	/**
	 * 設定したテクスチャが解放されていれば true（描かない）
	 *
	 * 理由はシェーダーごとに 1 回だけログに出す（毎フレーム出すと溢れる）。std::cout は GameMain が debug_log へ流す
	 */
	bool is_texture_lost( const TextureSlot& slot ) const
	{
		if ( ! slot.is_lost( * graphics_manager_ ) )
		{
			return false;
		}

		if ( ! texture_lost_logged_ )
		{
			texture_lost_logged_ = true;
			std::cout << "[Shader] " << get_name() << " : 設定したテクスチャが解放されたので描きません" << std::endl;
		}

		return true;
	}

	/// パラメーターの GUI で選んだ画像を読み込む
	Texture* load_texture_from_file( const char_t* file_path ) override { return graphics_manager_->load_texture( file_path ); }

	const ConstantBuffer* get_game_constant_buffer() const { return get_graphics_manager()->get_game_render_data(); }
	const ConstantBuffer* get_frame_constant_buffer() const { return get_graphics_manager()->get_frame_render_data(); }
	const ConstantBuffer* get_frame_drawing_constant_buffer() const { return get_graphics_manager()->get_frame_drawing_render_data(); }
	const ConstantBuffer* get_object_constant_buffer() const { return get_graphics_manager()->get_current_object_constant_buffer(); }
	const ConstantBuffer* get_skining_constant_buffer() const { return get_graphics_manager()->get_current_skinning_constant_buffer(); }

	const ParameterInfoList* get_parameter_info_list() const { return nullptr; }

	int_t get_int( const char_t* ) const override { return 0; }
	int_t get_int_at( uint_t ) const override { return 0; }

	void set_int( const char_t*, int_t ) override { }
	void set_int_at( uint_t, int_t ) override { }

	float_t get_float( const char_t* ) const override { return 0.f; }
	float_t get_float_at( uint_t ) const override { return 0.f; }

	void set_float( const char_t*, float_t ) override { }
	void set_float_at( uint_t, float_t ) override { }

	Vector get_vector( const char_t* ) const override { return Vector::Zero; }
	Vector get_vector_at( uint_t ) const override { return Vector::Zero; }

	void set_vector( const char_t*, const Vector& ) override { }
	void set_vector_at( uint_t, const Vector& ) override { }

	Color get_color( const char_t* ) const override { return Color::Black; }
	Color get_color_at( uint_t ) const override { return Color::Black; }

	void set_color( const char_t*, const Color& ) override { }
	void set_color_at( uint_t, const Color& ) override { }

	Texture* get_texture( const char_t* ) override { return nullptr; }
	const Texture* get_texture( const char_t* ) const override { return nullptr; }

	void set_texture( const char_t*, Texture* ) override { }

public:
	explicit BaseShader( GraphicsManager* graphics_manager )
		: graphics_manager_( graphics_manager )
	{ }

	void update() const override { }

	void render_model( const core::graphics::Model* m ) const override
	{
		m->get_mesh()->bind();

		for ( uint_t n = 0; n < m->get_mesh()->get_rendering_vertex_group_count(); n++ )
		{
			render( m->get_mesh(), n );
		}
	}
};

} // namespace blue_sky::graphics::shader
