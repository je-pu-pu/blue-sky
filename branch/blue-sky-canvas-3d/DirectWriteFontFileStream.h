#ifndef DIRECT_WRITE_FONT_FILE_STREAM_H
#define DIRECT_WRITE_FONT_FILE_STREAM_H

#include "DirectX.h"
#include <dwrite.h>

#include <fstream>
#include <vector>

class DirectWriteFontFileStream : public IDWriteFontFileStream
{
private:
    ULONG					ref_count_;
	UINT64					file_size_;

	
	std::vector< char >		file_content_;
	std::vector< char >		fragment_buffer_;

public:
	DirectWriteFontFileStream( const char* file_path )
		: ref_count_( 0 )
		, file_size_( 0 )
    {
		std::ifstream ifs( file_path, std::ios::in | std::ios::binary );

		std::ios::pos_type current_pos = ifs.tellg();

		ifs.seekg( 0, std::ios::end );

		file_size_ = ifs.tellg();

		ifs.seekg( 0 );

		file_content_.resize( static_cast< std::vector< char >::size_type >( file_size_ ) );
		
		ifs.read( & file_content_[ 0 ], file_size_ );

		ifs.close();
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID id, void** object )
	{
		if ( id == IID_IUnknown || id == __uuidof( IDWriteFontFileStream ) )
		{
			*object = this;
			AddRef();
			return S_OK;
		}
		else
		{
			*object = 0;
			return E_NOINTERFACE;
		}
	}
    
	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return InterlockedIncrement( & ref_count_ );
	}

    virtual ULONG STDMETHODCALLTYPE Release()
	{
		ULONG new_count = InterlockedDecrement( & ref_count_ );

		if ( new_count == 0 )
		{
			delete this;
		}

		return new_count;
	}

	virtual HRESULT STDMETHODCALLTYPE ReadFileFragment( void const** start, UINT64 offset, UINT64 size, OUT void** context )
	{
		*context = 0;

		if ( file_content_.empty() )
		{
			return E_FAIL;
		}

		if ( offset + size > file_size_ )
		{
			*start = 0;

			return E_FAIL;
		}

		*start = & file_content_[ static_cast< std::vector< char >::size_type >( offset ) ];

		return S_OK;
	}

    virtual void STDMETHODCALLTYPE ReleaseFileFragment( void* context )
	{
		
	}

    virtual HRESULT STDMETHODCALLTYPE GetFileSize( OUT UINT64* file_size )
	{
		*file_size = file_size_;

		return S_OK;
	}

    virtual HRESULT STDMETHODCALLTYPE GetLastWriteTime( OUT UINT64* last_write_time )
	{
		*last_write_time = 0;

		return E_NOTIMPL;
	}

}; // class DirectWriteFontFileStream

#endif // DIRECT_WRITE_FONT_FILE_STREAM_H