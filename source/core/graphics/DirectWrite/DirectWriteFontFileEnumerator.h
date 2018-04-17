#ifndef DIRECT_WRITE_FONT_FILE_ENUMRATOR_H
#define DIRECT_WRITE_FONT_FILE_ENUMRATOR_H

#include <core/DirectX.h>
#include "DirectWriteFontFileLoader.h"

class DirectWriteFontFileEnumerator : public IDWriteFontFileEnumerator
{
private:
    ULONG				ref_count_;
	IDWriteFactory*		factory_;
	IDWriteFontFile*	current_font_file_;

	const char*			file_path_;
public:
	DirectWriteFontFileEnumerator( IDWriteFactory* factory, const char* file_path )
		: ref_count_( 0 )
		, factory_( factory )
		, current_font_file_( 0 )
		, file_path_( file_path )
    {
		DIRECT_X_ADD_REF( factory_ );
    }

	~DirectWriteFontFileEnumerator()
	{
		DIRECT_X_RELEASE( current_font_file_ );
		DIRECT_X_RELEASE( factory_ );
	}

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID id, void** object )
	{
		if ( id == IID_IUnknown || id == __uuidof( IDWriteFontFileEnumerator ) )
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

    virtual HRESULT STDMETHODCALLTYPE MoveNext( OUT BOOL* has_current_file )
	{
		if ( current_font_file_ )
		{
			*has_current_file = FALSE;
			return S_OK;
		}
		
		const char* reference_key = file_path_;
		HRESULT hr = factory_->CreateCustomFontFileReference( reference_key, strlen( reference_key ) + 1, DirectWriteFontFileLoader::GetLoader(), & current_font_file_ );

		if ( FAILED( hr ) )
		{
			*has_current_file = FALSE;
		}

		*has_current_file = TRUE;

		return hr;
	}

    virtual HRESULT STDMETHODCALLTYPE GetCurrentFontFile( OUT IDWriteFontFile** font_file )
	{
		*font_file = current_font_file_;
		DIRECT_X_ADD_REF( current_font_file_ );

		return current_font_file_ ? S_OK : E_FAIL;
	}

}; // class DirectWriteFontFileEnumerator

#endif // DIRECT_WRITE_FONT_FILE_ENUMRATOR_H
