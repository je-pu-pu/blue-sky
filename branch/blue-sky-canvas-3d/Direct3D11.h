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
	ID3D11RenderTargetView*	render_target_view_;	///< Direct3D 11 Render Target View

	ID3DX11Effect*			effect_;

	// ?
	ID3D11InputLayout*      vertex_layout_;

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

	/** BAD functions */
	inline ID3D11Device* getDevice() { return device_; }
	inline ID3D11DeviceContext* getImmediateContext() { return immediate_context_; }
	inline IDXGISwapChain* getSwapChain() { return swap_chain_; }
	inline ID3D11RenderTargetView* getRenderTargetView() { return render_target_view_; }


}; // class Direct3D11

#endif // DIRECT_3D_11_H