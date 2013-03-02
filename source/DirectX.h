#ifndef DIRECT_X_H
#define DIRECT_X_H

#include <common/exception.h>

#include <windows.h>
#include <dxerr.h>
#pragma comment( lib, "dxerr.lib" )

template< typename T > inline void DIRECT_X_ADD_REF( T& x ) { if ( x ) { x->AddRef(); } }
template< typename T > inline void DIRECT_X_RELEASE( T& x ) { if ( x ) { x->Release(); } x = 0; }

#define DIRECT_X_FAIL_CHECK( x ) { HRESULT hresult = x; if ( FAILED( hresult ) ) { throw common::exception< HRESULT >( __FILE__, __LINE__, hresult ); } }

template< typename T >
class com_ptr
{
private:
	T*		ptr_;

public:
	com_ptr()
		: ptr_( 0 )
	{
		//
	}

	com_ptr( T* ptr )
		: ptr_( ptr )
	{
		//
	}

	~com_ptr()
	{
		DIRECT_X_RELEASE( ptr_ );
	}

	inline com_ptr& operator = ( T* ptr ) { assert( ! ptr_ ); ptr_ = ptr; return *this; }
	
	inline T** operator & () { return & ptr_; }

	inline T* operator -> () { return ptr_; }
	inline const T* operator -> () const { return ptr_; }

	inline T* get() { return ptr_; }
	inline T* get() const { return ptr_; }
}; 

#endif // DIRECT_X_H
