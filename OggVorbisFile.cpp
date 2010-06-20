#include "OggVorbisFile.h"
#include <common/exception.h>

#include <vorbis/vorbisfile.h>

#pragma comment ( lib, "libogg_static.lib" )
#pragma comment ( lib, "libvorbis_static.lib" )
#pragma comment ( lib, "libvorbisfile_static.lib" )

OggVorbisFile::OggVorbisFile( const char* file_name )
{
	OggVorbis_File file;

	int error = ov_fopen( const_cast< char* >( file_name ), & file );

	if ( error != 0 )
	{
		COMMON_THROW_EXCEPTION;
	}

	ov_clear( & file );
}

OggVorbisFile::~OggVorbisFile()
{
	
}