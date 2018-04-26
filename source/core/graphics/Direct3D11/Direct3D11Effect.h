#pragma once

#include "Direct3D11EffectTechnique.h"
#include "Direct3D11EffectPass.h"

#include <map>

class Direct3D11;
class Direct3D11EffectTechnique;
class Direct3D11EffectPass;

struct ID3DX11Effect;

/**
 * Direct3D 11 Effect のラッパークラス
 *
 */
class Direct3D11Effect
{
public:
	typedef Direct3D11					Direct3D;

	typedef Direct3D11EffectTechnique	Technique;
	typedef Direct3D11EffectPass		Pass;

	typedef std::map< const char*, Technique* > TechniqueList;

private:
	Direct3D*			direct_3d_;
	ID3DX11Effect*		effect_;

	TechniqueList		technique_list_;

public:
	Direct3D11Effect( Direct3D* );
	~Direct3D11Effect();
	
	void load( const char* );

	Technique* getTechnique( const char* name );

}; // class Direct3D11Effect
