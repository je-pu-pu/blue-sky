#ifndef DIRECT_WRITE_FONT_FILE_LOADER_H
#define DIRECT_WRITE_FONT_FILE_LOADER_H

#include "DirectWriteFontFileStream.h"

class DirectWriteFontFileLoader : public IDWriteFontFileLoader
{
private:
    ULONG ref_count_;

public:
	DirectWriteFontFileLoader()
		: ref_count_( 0 )
    {

    }

	~DirectWriteFontFileLoader()
    {

    }
	
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID id, void** object )
	{
		if ( id == IID_IUnknown || id == __uuidof( IDWriteFontFileLoader ) )
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

	virtual HRESULT STDMETHODCALLTYPE CreateStreamFromKey( const void* file_path, UINT32 file_path_length, OUT IDWriteFontFileStream** font_file_stream );

	static IDWriteFontFileLoader* GetLoader();

}; // class DirectWriteFontFileLoader

#endif // DIRECT_WRITE_FONT_FILE_LOADER_H
