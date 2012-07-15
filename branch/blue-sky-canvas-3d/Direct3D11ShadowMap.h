#ifndef DIRECT_3D_11_SHADOW_MAP_H
#define DIRECT_3D_11_SHADOW_MAP_H

#include <d3d11.h>
#include <xnamath.h>

class Direct3D11;
class Direct3D11ConstantBuffer;

/**
 * Direct3D 11 によるシャドウマップ
 *
 */
class Direct3D11ShadowMap
{
public:

private:
	Direct3D11*					direct_3d_;
	
	ID3D11Texture2D*			depth_stencil_texture_;
	ID3D11DepthStencilView*		depth_stencil_view_;

	ID3D11ShaderResourceView*	shader_resource_view_;

	D3D11_VIEWPORT				viewport_;

	static XMMATRIX					view_matrix_;
	static XMMATRIX					projection_matrix_;

public:
	Direct3D11ShadowMap( Direct3D11*, size_t );
	virtual ~Direct3D11ShadowMap();

	void render();

	ID3D11ShaderResourceView* getShaderResourceView() { return shader_resource_view_; }

	const XMMATRIX& getViewMatrix() const { return view_matrix_; }
	const XMMATRIX& getProjectionMatrix() const { return projection_matrix_; }
	
}; // class Direct3D11ShadowMap

#endif // DIRECT_3D_11_SHADOW_MAP_H