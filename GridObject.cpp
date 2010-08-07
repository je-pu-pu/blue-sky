#include "GridObject.h"

#include "GridData.h"
#include "Direct3D9Mesh.h"

#include <common/math.h>

namespace blue_sky
{

GridObject::GridObject( int x, int y, int z, int r, GridData* grid_data )
	: x_( x )
	, y_( y )
	, z_( z )
	, rotate_degree_( r )
	, grid_data_( grid_data )
	, lod_( 1 )
	, last_lod_( 1 )
	, visible_alpha_( 0.f )
	, lod_alpha_( 0.f )
{
	color_[ 0 ] = color_[ 1 ] = color_[ 2 ] = color_[ 3 ] = 1.f;
}

GridObject::~GridObject()
{
	
}

void GridObject::set_visible( bool visible )
{
	float speed = 0.01f;

	if ( visible )
	{
		visible_alpha_ = math::chase( visible_alpha_, 1.f, speed );
	}
	else
	{
		visible_alpha_ = math::chase( visible_alpha_, 0.f, speed );
	}
}

void GridObject::set_lod( int lod )
{
	if ( lod != lod_ && grid_data_->mesh( lod ) != grid_data_->mesh( lod_ ) )
	{
		last_lod_ = lod_;
		lod_alpha_ = 0.f;
	}

	lod_ = lod;

	if ( last_lod_ != lod_ )
	{
		float speed = 0.01f;
		lod_alpha_ = math::chase( lod_alpha_, 1.f, speed );

		if ( lod_alpha_ == 1.f )
		{
			last_lod_ = lod_;
		}
	}
}

int GridObject::width() const { return grid_data_->width(); }
int GridObject::depth() const { return grid_data_->depth(); }

void GridObject::render() const
{
	grid_data_->mesh( lod_ )->render();
}

void GridObject::render_last_lod() const
{
	grid_data_->mesh( last_lod_ )->render();
}

}; // namespace blue_sky