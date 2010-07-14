#include "GridData.h"
#include "GridCell.h"

#include "GameMain.h"
#include "Direct3D9Mesh.h"

#include <common/serialize.h>
#include <common/exception.h>

#include <boost/algorithm/string.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>

namespace blue_sky
{

GridCell GridData::null_cell_( 0, 0 ); 

GridData::GridData( )
	: width_( 0 )
	, depth_( 0 )
	, cell_( 0 )
{
	for ( int n = 0; n < LOD_MAX; n++ )
	{
		mesh_[ n ] = 0;
	}
}

GridData::GridData( int w, int d )
	: width_( w )
	, depth_( d )
	, cell_( new GridCell[ width_ * depth_ ] )
{
	for ( int n = 0; n < LOD_MAX; n++ )
	{
		mesh_[ n ] = 0;
	}
}

GridData::~GridData()
{
	delete [] cell_;

	for ( int n = 0; n < LOD_MAX; n++ )
	{
		delete mesh_[ n ];
	}
}

GridData* GridData::load_file( const char* file_name )
{
	GridData* grid_data = new GridData();

	std::ifstream in( file_name );
	
	if ( ! in.good() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "load grid data file \"" ) + file_name + "\" failed." );
	}

	while ( in.good() )
	{
		std::string line;
		std::getline( in, line );

		std::stringstream ss;
		
		std::string name;
		
		ss << line;
		ss >> name;

		if ( name == "width" )
		{
			ss >> grid_data->width_;
		}
		else if ( name == "depth" )
		{
			ss >> grid_data->depth_;
		}
		else if ( name == "mesh" )
		{
			int lod = 0;
			std::string mesh_file_name;

			ss >> lod;
			ss >> mesh_file_name;

			if ( lod < 0 )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( std::string( "\"mesh lod" ) + "\" is not valid, in " + file_name );
			}
			if ( lod >= LOD_MAX )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( std::string( "\"mesh lod" ) + "\" is not valid, in " + file_name );
			}
			if ( grid_data->mesh_[ lod ] )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( std::string( "\"mesh " ) + common::serialize( lod ) + "\" is already defined, in " + file_name );
			}

			grid_data->mesh_[ lod ] = load_mesh( mesh_file_name.c_str() );

			if ( ! grid_data->mesh_ )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( std::string( "load mesh file \"" ) + mesh_file_name + "\" failed, in " + file_name );
			}
		}
		else if ( name == "height" )
		{
			if ( ! grid_data->cell_ )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( std::string( "\"width\" or \"depth\" is zero, in " ) + file_name );
			}

			for ( int z = grid_data->depth_ - 1; z >= 0; z-- )
			{
				std::string height_line;
				std::getline( in, height_line );

				std::stringstream height_ss;
				height_ss << height_line;

				for ( int x = 0; x < grid_data->width_; x++ )
				{
					int height = 0;
					height_ss >> height;

					grid_data->cell( x, z ).height() = height;
				}
			}
		}
		else if ( name == "bound" )
		{
			if ( ! grid_data->cell_ )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( std::string( "\"width\" or \"depth\" is zero, in " ) + file_name );
			}

			for ( int z = grid_data->depth_ - 1; z >= 0; z-- )
			{
				std::string bound_line;
				std::getline( in, bound_line );

				std::stringstream bound_ss;
				bound_ss << bound_line;

				for ( int x = 0; x < grid_data->width_; x++ )
				{
					int bound = 0;	
					bound_ss >> bound;

					grid_data->cell( x, z ).bound() = bound;
				}
			}
		}

		if ( ! grid_data->cell_ && grid_data->width_ > 0 && grid_data->depth_ > 0 )
		{
			grid_data->cell_  = new GridCell[ grid_data->width_ * grid_data->depth_ ];
		}
	}

	std::vector< std::string > name_list;
	boost::algorithm::split( name_list, file_name, boost::algorithm::is_any_of( std::string( "\\/" ) ) );

	grid_data->set_name( name_list[ name_list.size() - 1 ].c_str() );

	return grid_data;
}

const GridData::Mesh* GridData::load_mesh( const char* file_name )
{
	/// @todo GameMain ‚ðŽQÆ‚µ‚È‚¢ ( GridData::load_file( "name", GameMain::load_mesh( this ) ) )
	/// @todo MeshManager ‚ªŠÇ—‚·‚éH

	Mesh* mesh = new Direct3D9Mesh( GameMain::getInstance()->get_direct_3d() );
	mesh->load_x( ( std::string( "media/model/" ) + file_name ).c_str() );

	return mesh;
}

GridCell& GridData::cell( int x, int z )
{
	assert( x >= 0 );
	assert( z >= 0 );
	assert( x < width_ );
	assert( z < depth_ );

	return cell_[ z * width_ + x ];
}

const GridCell& GridData::cell( int x, int z ) const
{
	if ( x < 0 ) return null_cell_;
	if ( z < 0 ) return null_cell_;
	if ( x >= width_ ) return null_cell_;
	if ( z >= depth_ ) return null_cell_;

	return cell_[ z * width_ + x ];
}

void GridData::put( int px, int py, int pz, int r, const GridData* grid_data )
{
	int xddx = 1;
	int xddz = 0;
	int zddx = 0;
	int zddz = 1;
	bool dx_reset_at_x_loop_end = true;
	bool dz_reset_at_x_loop_end = false;

	r %= 360;

	if ( r == 90 )
	{
		pz -= 1;
		xddx = 0;
		xddz = -1;
		zddx = 1;
		zddz = 0;
		dx_reset_at_x_loop_end = false;
		dz_reset_at_x_loop_end = true;
	}
	else if ( r == 180 )
	{
		px -= 1;
		pz -= 1;
		xddx = -1;
		xddz = 0;
		zddx = 0;
		zddz = -1;
	}
	else if ( r == 270 )
	{
		px -= 1;
		xddx = 0;
		xddz = 1;
		zddx = -1;
		zddz = 0;
		dx_reset_at_x_loop_end = false;
		dz_reset_at_x_loop_end = true;
	}

	int dx = px;
	int dz = pz;

	for ( int z = 0; z < grid_data->depth(); z++, dx += zddx, dz += zddz )
	{
		for ( int x = 0; x < grid_data->width(); x++, dx += xddx, dz += xddz )
		{
			if ( dx < 0 ) continue;
			if ( dx >= width() ) continue;
			if ( dz < 0 ) continue;
			if ( dz >= depth() ) continue;

			GridCell& target_cell = cell( dx, dz );

			if ( target_cell.height() > static_cast< int >( py + grid_data->cell( x, z ).height() ) )
			{
				continue;
			}

			target_cell.bound() = grid_data->cell( x, z ).bound();
			target_cell.height() = py + grid_data->cell( x, z ).height();
		}

		if ( dx_reset_at_x_loop_end ) dx = px;
		if ( dz_reset_at_x_loop_end ) dz = pz;
	}
}

}; // namespace blue_sky