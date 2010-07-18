#include "Direct3D9Resource.h"
#include "Direct3D9.h"

Direct3D9Resource::Direct3D9Resource( Direct3D9* direct_3d )
	: direct_3d_( direct_3d )
{
	direct_3d_->add_resource( this );
}

Direct3D9Resource::~Direct3D9Resource()
{
	direct_3d_->remove_resource( this );
}