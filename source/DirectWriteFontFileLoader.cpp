#include "DirectWriteFontFileLoader.h"
#include <new>

#include "memory.h"

IDWriteFontFileLoader* DirectWriteFontFileLoader::GetLoader()
{
	static IDWriteFontFileLoader* instance = new DirectWriteFontFileLoader();

	return instance;
}

HRESULT STDMETHODCALLTYPE DirectWriteFontFileLoader::CreateStreamFromKey( const void* file_path, UINT32 file_path_length, OUT IDWriteFontFileStream** font_file_stream )
{
	*font_file_stream = new DirectWriteFontFileStream( static_cast< const char* >( file_path ) );
	DIRECT_X_ADD_REF( *font_file_stream );

	return S_OK;
}