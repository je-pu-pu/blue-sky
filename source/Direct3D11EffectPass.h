#ifndef DIRECT_3D_11_EFFECT_PASS_H
#define DIRECT_3D_11_EFFECT_PASS_H

#include <d3d11.h>

class Direct3D11;

struct ID3DX11EffectPass;

/**
 * Direct3D 11 Effect Pass のラッパークラス
 *
 */
class Direct3D11EffectPass
{
public:
	typedef Direct3D11				Direct3D;

	typedef ID3D11InputLayout		InputLayout;

private:
	Direct3D*						direct_3d_;			
	ID3DX11EffectPass*				pass_;

public:
	Direct3D11EffectPass( Direct3D*, ID3DX11EffectPass* );
	~Direct3D11EffectPass();
	
	InputLayout* create_input_layout( D3D11_INPUT_ELEMENT_DESC[], UINT );

	void apply();

}; // class Direct3D11EffectPass

#endif // DIRECT_3D_11_EFFECT_PASS_H