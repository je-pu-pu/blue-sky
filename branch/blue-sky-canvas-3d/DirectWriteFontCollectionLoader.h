#ifndef DIRECT_WRITE_FONT_COLLECTION_LOADER_H
#define DIRECT_WRITE_FONT_COLLECTION_LOADER_H

#include "DirectWriteFontFileEnumerator.h"
#include <new>

class DirectWriteFontCollectionLoader : public IDWriteFontCollectionLoader
{
private:
    ULONG ref_count_;

	DirectWriteFontCollectionLoader()
		: ref_count_( 0 )
    {

    }

	~DirectWriteFontCollectionLoader()
    {

    }

public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID id, void** object )
	{
		if ( id == IID_IUnknown || id == __uuidof( IDWriteFontCollectionLoader ) )
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

    virtual HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey( IDWriteFactory* factory, const void* file_path, UINT32 file_path_length, OUT IDWriteFontFileEnumerator** font_file_enumrator )
	{
		*font_file_enumrator = 0;
		
		DirectWriteFontFileEnumerator* enumerator = new( std::nothrow ) DirectWriteFontFileEnumerator( factory, static_cast< const char* >( file_path ) );
		
		if ( ! enumerator )
		{
			return E_OUTOFMEMORY;
		}

		DIRECT_X_ADD_REF( enumerator );
		*font_file_enumrator = enumerator;

	    return S_OK;
	}

    static IDWriteFontCollectionLoader* GetLoader()
    {
		static DirectWriteFontCollectionLoader* instance = new DirectWriteFontCollectionLoader();

        return instance;
    }
}; // class DirectWriteFontCollectionLoader

#endif // DIRECT_WRITE_FONT_COLLECTION_LOADER_H