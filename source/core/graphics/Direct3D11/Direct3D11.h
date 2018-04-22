#ifndef DIRECT_3D_11_H
#define DIRECT_3D_11_H

#include <core/DirectX.h>
#include <core/type.h>

#include <common/auto_ptr.h>

#include <d3d11.h>

#include <map>

class Direct3D11Vector;
class Direct3D11Matrix;
class Direct3D11Color;
class Direct3D11Texture;

class Direct3D11Sprite;
class Direct3D11Fader;

class Direct3D11Effect;
class Direct3D11EffectTechnique;
class Direct3D11EffectPass;

class Direct3D11MeshManager;
class Direct3D11TextureManager;

class DirectWrite;

struct ID3DX11Effect;

/**
 * Direct3D 11 �̃��b�p�[�N���X
 *
 */
class Direct3D11
{
public:
	typedef Direct3D11Sprite			Sprite;
	typedef Direct3D11Fader				Fader;

	typedef Direct3D11Effect			Effect;
	typedef Direct3D11EffectTechnique	EffectTechnique;
	typedef Direct3D11EffectPass		EffectPass;

	typedef Direct3D11MeshManager		MeshManager;
	typedef Direct3D11TextureManager	TextureManager;

	typedef DirectWrite					Font;

	typedef ID3D11InputLayout			InputLayout;
	
	typedef Direct3D11Vector			Vector;
	typedef Direct3D11Matrix			Matrix;
	typedef Direct3D11Color				Color;
	typedef Direct3D11Texture			Texture;

	typedef std::map< const char*, InputLayout* >		InputLayoutList;

private:
	static const DXGI_FORMAT DEPTH_STENCIL_FORMAT = DXGI_FORMAT_D32_FLOAT;
	static const Color DEFAULT_CLEAR_COLOR;

	com_ptr< IDXGIAdapter1 >	dxgi_adapter_;

	/// @todo �X�}�[�g�|�C���^ ( com_ptr ) ������H
	ID3D11Device*				device_;				///< Direct3D 11 Device
	ID3D11DeviceContext*		immediate_context_;		///< Direct3D 11 Device Context
	IDXGISwapChain*				swap_chain_;			///< Direct3D 11 Swap Chain

	DXGI_SWAP_CHAIN_DESC		swap_chain_desc_;

	ID3D11Texture2D*			back_buffer_texture_;
	ID3D11RenderTargetView*		back_buffer_view_;		///< Direct3D 11 Render Target View

	ID3D11Texture2D*			depth_stencil_texture_;
	ID3D11DepthStencilView*		depth_stencil_view_;

	InputLayoutList				vertex_layout_list_;

	D3D11_VIEWPORT				viewport_;

	// for Direct2D & DirectWrite
	ID3D10Device1*				device_10_;

	IDXGISurface1*				back_buffer_surface_;

	ID3D11Texture2D*			text_texture_;
	common::auto_ptr< Texture >	text_view_;

	IDXGISurface1*				text_surface_;
	IDXGIKeyedMutex*			text_texture_mutex_11_;
	IDXGIKeyedMutex*			text_texture_mutex_10_;
	UINT64						text_texture_sync_key_ = 0;

	common::auto_ptr< Font	>				font_;

	common::auto_ptr< Sprite >				sprite_;
	common::auto_ptr< Fader >				fader_;
	common::auto_ptr< Effect >				effect_;

	common::auto_ptr< MeshManager >			mesh_manager_;
	common::auto_ptr< TextureManager >		texture_manager_;

	void create_device();
	void create_swap_chain( IDXGIFactory1*, HWND, uint_t, uint_t, bool, int, int );
	void create_back_buffer_view();
	void create_back_buffer_surface();
	void create_depth_stencil_view();
	void setup_viewport();

	void log_all_adapter_desc( IDXGIFactory1* );
	void log_adapter_desc( int, const DXGI_ADAPTER_DESC1& );
	void log_feature_level();

	void text_out_device_caps( const char*, bool = false );

public:
	Direct3D11( HWND, int, int, bool, const char* = 0, const char* = 0, int = 0, int = 0 );
	~Direct3D11();
	
	ID3D11RenderTargetView* create_render_target_view( ID3D11Texture2D* );
	ID3D11DepthStencilView* create_depth_stencil_view( ID3D11Texture2D* );

	void setup_font();

	void create_default_input_layout();
	void create_input_layout( char_t*, char_t*, D3D11_INPUT_ELEMENT_DESC[], UINT );

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

	void set_default_render_target();
	void set_default_viewport();

	void set_render_target_for_vr( ID3D11RenderTargetView*, ID3D11DepthStencilView* );

	void unset_render_target();

	void set_viewport_for_left_eye();
	void set_viewport_for_right_eye();

	void setInputLayout( const char* );

	void bind_texture_to_ps( uint_t, const Texture* );


	void begin2D();
	void end2D();
	void begin3D();
	void end3D();

	void present();

	void renderText();

	void setDebugViewport( float, float, float, float );

	void getTexture2dDescByTexture( const Texture*, D3D11_TEXTURE2D_DESC* );

	inline Font* getFont() { return font_.get(); }
	inline Sprite* getSprite() { return sprite_.get(); }
	inline Fader* getFader() { return fader_.get(); }
	inline Effect* getEffect() { return effect_.get(); }
	inline MeshManager* getMeshManager() { return mesh_manager_.get(); }
	inline TextureManager* getTextureManager() { return texture_manager_.get(); }

	inline const Font* getFont() const { return font_.get(); }
	inline const Sprite* getSprite() const { return sprite_.get(); }
	inline const Fader* getFader() const { return fader_.get(); }
	inline const Effect* getEffect() const { return effect_.get(); }
	inline const MeshManager* getMeshManager() const { return mesh_manager_.get(); }
	inline const TextureManager* getTextureManager() const { return texture_manager_.get(); }

	/** BAD functions */
	inline ID3D11Device* getDevice() const { return device_; }
	inline ID3D11DeviceContext* getImmediateContext() const { return immediate_context_; }

	inline IDXGISurface1* getBackbufferSurface() const { return back_buffer_surface_; }

}; // class Direct3D11

#endif // DIRECT_3D_11_H