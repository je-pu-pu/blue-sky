#pragma once

#include "Direct3D11Mesh.h"

/**
 * Direct3D 11 Rectangle
 *
 */
class Direct3D11Rectangle : public Direct3D11Mesh 
{
private:
	Material* get_material_at( uint_t n, bool force ) = delete;
	/*
	{
		return Direct3D11Mesh::get_material_at( n, force );
	}
	*/

	MaterialList& get_material_list() = delete;
	const MaterialList& get_material_list() const = delete;

public:
	Direct3D11Rectangle( Direct3D11* );
	virtual ~Direct3D11Rectangle() { }

	Material* get_material() { return Direct3D11Mesh::get_material_at( 0 ); }
	const Material* get_material() const { return Direct3D11Mesh::get_material_at( 0 ); }

}; // class Direct3D11Rectangle
