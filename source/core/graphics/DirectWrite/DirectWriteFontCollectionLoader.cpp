#include "DirectWriteFontCollectionLoader.h"
#include <new>

IDWriteFontCollectionLoader* DirectWriteFontCollectionLoader::GetLoader()
{
	static IDWriteFontCollectionLoader* instance = new DirectWriteFontCollectionLoader();

	return instance;
}

HRESULT STDMETHODCALLTYPE DirectWriteFontCollectionLoader::CreateEnumeratorFromKey( IDWriteFactory* factory, const void* file_path, UINT32 /* file_path_length */, OUT IDWriteFontFileEnumerator** font_file_enumrator )
{
	*font_file_enumrator = 0;
		
	DirectWriteFontFileEnumerator* enumerator = new DirectWriteFontFileEnumerator( factory, static_cast< const char* >( file_path ) );
		
	if ( ! enumerator )
	{
		return E_OUTOFMEMORY;
	}

	DIRECT_X_ADD_REF( enumerator );
	*font_file_enumrator = enumerator;

	return S_OK;
}
