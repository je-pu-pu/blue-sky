#include "WaveFile.h"
#include <common/exception.h>

WaveFile::WaveFile( const char* file_name )
	: data_( 0 )
{
	HMMIO hmmio = mmioOpen( const_cast< LPSTR >( file_name ), 0, MMIO_READ );

	if( ! hmmio )
	{
		COMMON_THROW_EXCEPTION;
	}
	
	riff_chunk_.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' );

	if ( mmioDescend( hmmio, & riff_chunk_, 0, MMIO_FINDRIFF ) == MMIOERR_CHUNKNOTFOUND )
	{
		COMMON_THROW_EXCEPTION;
	}
	
	format_chunk_.ckid = mmioFOURCC( 'f', 'm', 't', ' ' );

	if ( mmioDescend( hmmio, & format_chunk_, & riff_chunk_, MMIO_FINDCHUNK ) == MMIOERR_CHUNKNOTFOUND )
	{
		COMMON_THROW_EXCEPTION;
	}	

	mmioRead( hmmio, reinterpret_cast< HPSTR >( & format_ ), format_chunk_.cksize );
	mmioAscend( hmmio, & format_chunk_, 0 );

	data_chunk_.ckid = mmioFOURCC( 'd', 'a', 't', 'a' );
	if ( mmioDescend( hmmio, & data_chunk_, & riff_chunk_, MMIO_FINDCHUNK ) == MMIOERR_CHUNKNOTFOUND )
	{
		COMMON_THROW_EXCEPTION;
	}

	data_ = new char[ data_chunk_.cksize ];

	mmioRead( hmmio, data_, data_chunk_.cksize );
	mmioClose( hmmio, 0 );
}

WaveFile::~WaveFile()
{
	delete [] data_;
}