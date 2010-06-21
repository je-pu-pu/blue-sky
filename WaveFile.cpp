#include "WaveFile.h"
#include <common/random.h>
#include <common/exception.h>

WaveFile::WaveFile( const char* file_name )
	: hmmio_( 0 )
{
	hmmio_ = mmioOpen( const_cast< LPSTR >( file_name ), 0, MMIO_READ );

	if( ! hmmio_ )
	{
		COMMON_THROW_EXCEPTION;
	}
	
	riff_chunk_.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' );

	if ( mmioDescend( hmmio_, & riff_chunk_, 0, MMIO_FINDRIFF ) == MMIOERR_CHUNKNOTFOUND )
	{
		COMMON_THROW_EXCEPTION;
	}
	
	format_chunk_.ckid = mmioFOURCC( 'f', 'm', 't', ' ' );

	if ( mmioDescend( hmmio_, & format_chunk_, & riff_chunk_, MMIO_FINDCHUNK ) == MMIOERR_CHUNKNOTFOUND )
	{
		COMMON_THROW_EXCEPTION;
	}	

	mmioRead( hmmio_, reinterpret_cast< HPSTR >( & format_ ), format_chunk_.cksize );
	mmioAscend( hmmio_, & format_chunk_, 0 );

	data_chunk_.ckid = mmioFOURCC( 'd', 'a', 't', 'a' );
	if ( mmioDescend( hmmio_, & data_chunk_, & riff_chunk_, MMIO_FINDCHUNK ) == MMIOERR_CHUNKNOTFOUND )
	{
		COMMON_THROW_EXCEPTION;
	}
}

WaveFile::~WaveFile()
{
	mmioClose( hmmio_, 0 );
}

DWORD WaveFile::read( void* data, DWORD size )
{
	return mmioRead( hmmio_, static_cast< char* >( data ), size );
}