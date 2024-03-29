#pragma once

#include <core/DirectX.h>
#include <core/type.h>

#include <d3d11.h>

#include <unordered_map>
#include <memory>

class DirectWrite;

namespace core::graphics
{
	class Sprite;
}

namespace core::graphics::direct_3d_11
{
	class Texture;
	class RenderTargetTexture;
	class BackBufferTexture;

	class InputLayout;

	class Effect;
	class EffectTechnique;
	class EffectPass;

	class Sprite;

/**
 * Direct3D 11 のラッパークラス
 *
 */
class Direct3D11
{
public:
	using Texture			= core::graphics::direct_3d_11::Texture;

	using InputLayout		= core::graphics::direct_3d_11::InputLayout;
	using InputLayoutList	= std::unordered_map< const char*, InputLayout* >;

	using Effect			= core::graphics::direct_3d_11::Effect;
	using EffectTechnique	= core::graphics::direct_3d_11::EffectTechnique;
	using EffectPass		= core::graphics::direct_3d_11::EffectPass;

	using Sprite			= core::graphics::Sprite;

	using Font				= DirectWrite;

	using Vector			= direct_x_math::Vector;
	using Matrix			= direct_x_math::Matrix;
	using Color				= direct_x_math::Color;

private:
	static const Color DEFAULT_CLEAR_COLOR;

	static Direct3D11* instance_;

	com_ptr< IDXGIAdapter1 >	dxgi_adapter_;

	/// @todo スマートポインタ ( com_ptr ) 化する？
	ID3D11Device*				device_;				///< Direct3D 11 Device
	ID3D11DeviceContext*		immediate_context_;		///< Direct3D 11 Device Context
	IDXGISwapChain*				swap_chain_;			///< Direct3D 11 Swap Chain

	DXGI_SWAP_CHAIN_DESC		swap_chain_desc_;

	std::unique_ptr< BackBufferTexture >	back_buffer_texture_;		///< バックバッファへの描画用テクスチャ

	ID3D11Texture2D*			depth_stencil_texture_;
	ID3D11DepthStencilView*		depth_stencil_view_;
	std::unique_ptr< Texture >	depth_texture_;

	InputLayoutList				input_layout_list_;

	D3D11_VIEWPORT				viewport_;

	// for Direct2D & DirectWrite
	ID3D10Device1*				device_10_;

	ID3D11Texture2D*			text_texture_;
	std::unique_ptr< Texture >	text_view_;

	IDXGISurface1*				text_surface_;
	IDXGIKeyedMutex*			text_texture_mutex_11_;
	IDXGIKeyedMutex*			text_texture_mutex_10_;
	UINT64						text_texture_sync_key_ = 0;

	std::unique_ptr< Font >		font_;

	std::unique_ptr< Sprite >	sprite_;
	std::unique_ptr< Effect >	effect_;

	void create_device();
	
	void create_swap_chain( IDXGIFactory1*, HWND, uint_t, uint_t, bool, int, int );
	
	void create_back_buffer_texture();

	void create_depth_stencil_view();
	void create_depth_texture();

	void setup_default_viewport();

	void log_all_adapter_desc( IDXGIFactory1* );
	void log_adapter_desc( int, const DXGI_ADAPTER_DESC1& );
	void log_feature_level();

	void text_out_device_caps( const char*, bool = false );

public:
	Direct3D11( HWND, int, int, bool, int = 0, int = 0 );
	~Direct3D11();

	inline static Direct3D11* get_instance() { return instance_; }
	
	/// OculusRift で使用
	ID3D11RenderTargetView* create_render_target_view( ID3D11Texture2D* );
	ID3D11DepthStencilView* create_depth_stencil_view( ID3D11Texture2D* );

	void setup_font();
	void setup_sprite();

	void create_default_input_layout();
	void create_input_layout( const char_t*, const char_t*, D3D11_INPUT_ELEMENT_DESC[], UINT );

	void reset( bool = false );

	int get_width() const { return swap_chain_desc_.BufferDesc.Width; }
	int get_height() const { return swap_chain_desc_.BufferDesc.Height; }

	void set_size( int, int );
	void set_full_screen( bool );
	void set_multi_sample( int, int );

	bool is_full_screen() const;
	void switch_full_screen();

	void on_resize( int, int );

	void clear_default_view( const Color& = DEFAULT_CLEAR_COLOR );

	void clear_back_buffer_view( const Color& );
	void clear_depth_stencil_view();

	void clear_render_target_view( ID3D11RenderTargetView*, const Color& = DEFAULT_CLEAR_COLOR );
	void clear_depth_stencil_view( ID3D11DepthStencilView* );

	void set_default_render_target( bool = true );
	void set_default_viewport();

	void set_render_target( RenderTargetTexture* );
	void set_render_target( BackBufferTexture* );

	void set_render_target_for_vr( ID3D11RenderTargetView*, ID3D11DepthStencilView* );

	void unset_render_target();

	void set_viewport( float_t, float_t, float_t, float_t, float_t = 0.f, float_t = 1.f );

	const InputLayout* get_input_layout( const char* ) const;
	void set_input_layout( const char* );
	void set_input_layout( const InputLayout* );

	[[nodiscard]] Texture* load_texture( const char* );

	void bind_texture_to_ds( uint_t, const Texture* );
	void bind_texture_to_ps( uint_t, const Texture* );


	void begin2D();
	void end2D();
	void begin3D();
	void end3D();

	void present();

	void renderText();

	inline Font* get_font() { return font_.get(); }
	inline Sprite* get_sprite() { return sprite_.get(); }
	inline Effect* get_effect() { return effect_.get(); }

	inline const Font* get_font() const { return font_.get(); }
	inline const Sprite* get_sprite() const { return sprite_.get(); }
	inline const Effect* get_effect() const { return effect_.get(); }

	BackBufferTexture* get_back_buffer_texture() { return back_buffer_texture_.get(); }

	void resolve_depth_texture();
	Texture* get_depth_texture() { return depth_texture_.get(); }

	/** BAD functions */
	inline ID3D11Device* getDevice() const { return device_; }
	inline ID3D11DeviceContext* getImmediateContext() const { return immediate_context_; }

}; // class Direct3D11

} // namespace core::graphics::direct_3d_11
