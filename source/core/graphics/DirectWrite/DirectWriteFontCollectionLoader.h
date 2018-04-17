#ifndef DIRECT_WRITE_FONT_COLLECTION_LOADER_H
#define DIRECT_WRITE_FONT_COLLECTION_LOADER_H

#include "DirectWriteFontFileEnumerator.h"

class DirectWriteFontCollectionLoader : public IDWriteFontCollectionLoader
{
private:
    ULONG ref_count_;

public:
	DirectWriteFontCollectionLoader()
		: ref_count_( 0 )
    {

    }

	~DirectWriteFontCollectionLoader()
    {

    }

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

    virtual HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey( IDWriteFactory* factory, const void* file_path, UINT32 file_path_length, OUT IDWriteFontFileEnumerator** font_file_enumrator );

    static IDWriteFontCollectionLoader* GetLoader();

}; // class DirectWriteFontCollectionLoader

#endif // DIRECT_WRITE_FONT_COLLECTION_LOADER_H