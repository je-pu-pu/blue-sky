#ifndef DIRECT_3D_9_SKY_BOX_H
#define DIRECT_3D_9_SKY_BOX_H

#include "Direct3D9Mesh.h"

/**
 * SkyBox
 *
 */
class Direct3D9SkyBox : public Direct3D9Mesh
{
private:
	const char* name_;
	const char* ext_;

	virtual std::string get_texture_file_name_by_texture_name( const char* ) const;

public:
	Direct3D9SkyBox( Direct3D9*, const char*, const char* = "png" );
	virtual ~Direct3D9SkyBox();

	void render();

}; // class Direct3D9SkyBox

#endif // DIRECT_3D_9_SKY_BOX_H
