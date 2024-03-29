#include "OggVorbisFile.h"

#include <common/exception.h>
#include <common/serialize.h>

OggVorbisFile::OggVorbisFile( const char* file_name )
{
	int error = ov_fopen( const_cast< char* >( file_name ), & file_ );

	if ( error != 0 )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "load ogg file \""  ) + file_name + "\", error code : " + common::serialize( error ) );
	}

	vorbis_info_ = ov_info( & file_, -1 );

	format_.wFormatTag = WAVE_FORMAT_PCM;
	format_.nChannels = static_cast< WORD >( vorbis_info_->channels );
	format_.nSamplesPerSec = vorbis_info_->rate;
    format_.nAvgBytesPerSec = 2 * vorbis_info_->rate * vorbis_info_->channels;
    format_.nBlockAlign = static_cast< WORD >( 2 * vorbis_info_->channels );
    format_.wBitsPerSample = 16;
    format_.cbSize = 0;
}

OggVorbisFile::~OggVorbisFile()
{
	ov_clear( & file_ );
}

OggVorbisFile::SizeType OggVorbisFile::size() const
{
	ogg_int64_t size = ov_pcm_total( const_cast< OggVorbis_File* >( & file_ ), -1 );

	if ( size == OV_EINVAL )
	{
		COMMON_THROW_EXCEPTION;
	}

	return static_cast< SizeType >( 2 * size * vorbis_info_->channels );
}

OggVorbisFile::SizeType OggVorbisFile::size_per_sec() const
{
	return format_.nAvgBytesPerSec;
}

OggVorbisFile::SizeType OggVorbisFile::read( void* data, SizeType size, bool loop )
{
	int bs = 0;
	SizeType read_bytes = 0;
	char* ptr = static_cast< char* >( data );

	while ( true )
	{
		long result = ov_read( & file_, ptr, size - read_bytes, 0, 2, 1, & bs );

		if ( result == 0 )
		{
			if ( int r = ov_raw_seek_lap( & file_, 0 ) != 0 )
			{
				COMMON_THROW_EXCEPTION;
			}

			if ( ! loop )
			{
				memset( ptr, 0, size - read_bytes );

				return size;
			}
		}
		else if ( result < 0 )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( std::string( "ov_read returns : " ) + common::serialize( result ) );
		}

		ptr += result;
		read_bytes += result;

		if ( read_bytes >= size )
		{
			break;
		}
	}

	return read_bytes;
}

void OggVorbisFile::seek( SizeType position )
{
	if ( int r = ov_raw_seek_lap( & file_, position ) != 0 )
	{
		COMMON_THROW_EXCEPTION;
	}
}
