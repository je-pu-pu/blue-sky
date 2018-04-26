#pragma once

#include "Direct3D11Mesh.h"

/**
 * SkyBox
 *
 */
class Direct3D11SkyBox : public Direct3D11Mesh
{
public:
	typedef Direct3D11 Direct3D;
	typedef Direct3D11Mesh Mesh;

private:
	Direct3D* direct_3d_;

	const char* name_;
	const char* ext_;

protected:
	virtual string_t get_texture_file_name_by_texture_name( const char* ) const;

public:
	Direct3D11SkyBox( Direct3D*, const char*, const char* = "png" );
	virtual ~Direct3D11SkyBox();

}; // class Direct3D11SkyBox
