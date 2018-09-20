#pragma once

#include "Direct3D11Common.h"
#include <core/type.h>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;

/**
 * デバッグ表示用の XYZ 軸
 *
 */
class Axis
{
public:
	using Position	= Vector3;
	using Color		= Vector3;

	struct Vertex
	{
		Position	Position;
		Color		Color;
	};

protected:
	Direct3D11*		direct_3d_;
	ID3D11Buffer*	vertex_buffer_;

	void create_vertex_buffer();

public:
	explicit Axis( Direct3D11* );
	virtual ~Axis();

	void render() const;

}; // class Axis

} // core::graphics::direct_3d_11
