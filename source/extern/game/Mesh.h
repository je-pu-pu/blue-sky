#pragma once

#include <type/type.h>
#include <vector>

namespace game
{

class Vertex { };
class VertexWeight;
class VertexGroup;
class Material;

/**
 * 抽象メッシュクラス
 *
 * 立体物の形状を保持する
 *
 */
class Mesh
{
public:
	using Vertex			= Vertex;
	using VertexWeight		= VertexWeight;
	using VertexWeightList	= std::vector< VertexWeight >;

public:
	Mesh() { }
	virtual ~Mesh() { }

	virtual void add_vertex( const Vertex& ) = 0;
	virtual uint_t get_vertex_count() const = 0;

	virtual void add_vertex_weight( const VertexWeight& ) = 0;

	virtual VertexGroup* create_vertex_group() = 0;
	virtual VertexGroup* get_vertex_group_at( uint_t ) = 0;
	
	virtual uint_t get_rendering_vertex_group_count() const = 0;

	virtual void invert_vertex_z() = 0;
	virtual void flip_polygon() = 0;

	virtual void bind() const = 0;
	virtual void render( uint_t ) const = 0;
};

/**
 * 頂点毎のウエイト情報
 *
 */
class VertexWeight
{
public:
	using BoneIndexList	= u8_t[ 4 ];
	using WeightList	= u8_t[ 4 ];
		
private:
	BoneIndexList	bone_index_list_ = { 0 };	///< ボーンインデックス ( 0 .. 3 )
	WeightList		weight_list_ = { 0 };		///< ウエイト ( 0 .. 3 )
	
public:
	VertexWeight()
	{

	}

	void add( int bone_index, float weight )
	{
		assert( bone_index >= 0 );
		assert( bone_index <= 255 );
		assert( weight >= 0.f );
		assert( weight <= 1.f );

		for ( int n = 0; n < 4; ++n )
		{
			if ( bone_index_list_[ n ] == 0 && weight_list_[ n ] == 0 )
			{
				bone_index_list_[ n ] = static_cast< u8_t >( bone_index );
				weight_list_[ n ] = static_cast< u8_t >( weight * 255 );

				return;
			}
		}

		assert( false );
	}
};

} // namespace game
