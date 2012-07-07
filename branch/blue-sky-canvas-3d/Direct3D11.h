#ifndef DIRECT_3D_11_H
#define DIRECT_3D_11_H

#include <common/safe_ptr.h>
#include <d3dx11.h>
#include <list>

struct ID3DX11Effect;

/**
 * Direct3D 11 のラッパークラス
 *
 */
class Direct3D11
{
public:

private:
	ID3D11Device*			device_;				///< Direct3D 11 Device
	ID3D11DeviceContext*	immediate_context_;		///< Direct3D 11 Device Context
	IDXGISwapChain*			swap_chain_;			///< Direct3D 11 Swap Chain

	ID3D11Texture2D*		back_buffer_texture_;
	ID3D11RenderTargetView*	back_buffer_view_;		///< Direct3D 11 Render Target View

	ID3D11Texture2D*		depth_stencil_texture_;
	ID3D11DepthStencilView*	depth_stencil_view_;

	ID3DX11Effect*			effect_;

	ID3D11InputLayout*      vertex_layout_;

	ID3D11Buffer*			constant_buffer_;

	// for Direct2D & DirectWrite
	ID3D10Device1*				device_10_;

	IDXGISurface1*				back_buffer_surface_;

	ID3D11Texture2D*			text_texture_;
	ID3D11ShaderResourceView*	text_view_;

	IDXGISurface1*				text_surface_;
	IDXGIKeyedMutex*			text_texture_mutex_11_;
	IDXGIKeyedMutex*			text_texture_mutex_10_;

	void text_out_adapter_info( const char*, bool = false );
	void text_out_device_caps( const char*, bool = false );

public:
	Direct3D11( HWND, int, int, bool, const char* = 0, const char* = 0, int = 0, int = 0 );
	~Direct3D11();
	
	void load_effect_file( const char* );
	void apply_effect();

	void reset( bool = false );

	void set_size( int, int );
	void set_full_screen( bool );
	void set_multi_sample( int, int );

	void set_depth_stencil( bool );

	void clear();

	void begin2D();
	void end2D();
	void begin3D();
	void end3D();

	void renderText();

	/** BAD functions */
	inline ID3D11Device* getDevice() { return device_; }
	inline ID3D11DeviceContext* getImmediateContext() { return immediate_context_; }
	inline ID3DX11Effect* getEffect() { return effect_; }

	inline IDXGISurface1* getBackbufferSurface() { return back_buffer_surface_; }

	inline IDXGISurface1* getTextSurface() { return text_surface_; }
	inline ID3D11ShaderResourceView* getTextView() { return text_view_; }

}; // class Direct3D11

#endif // DIRECT_3D_11_H