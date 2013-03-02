#ifndef DIRECT_3D_11_EFFECT_TECHNIQUE_H
#define DIRECT_3D_11_EFFECT_TECHNIQUE_H

#include <vector>

class Direct3D11;
class Direct3D11Effect;
class Direct3D11EffectPass;

struct ID3DX11EffectTechnique;

/**
 * Direct3D 11 Effect Technique のラッパークラス
 *
 */
class Direct3D11EffectTechnique
{
public:
	typedef Direct3D11					Direct3D;

	typedef Direct3D11Effect			Effect;
	typedef Direct3D11EffectPass		Pass;

	typedef std::vector< Pass* >		PassList;

private:
	Effect*				effect_;
	PassList			pass_list_;

public:
	Direct3D11EffectTechnique( Direct3D*, ID3DX11EffectTechnique* );
	~Direct3D11EffectTechnique();
	
	PassList& getPassList() { return pass_list_; }

}; // class Direct3D11EffectTechinique

#endif // DIRECT_3D_11_EFFECT_TECHNIQUE_H