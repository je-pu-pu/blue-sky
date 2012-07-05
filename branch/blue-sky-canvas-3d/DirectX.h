#ifndef DIRECT_X_H
#define DIRECT_X_H

#include <common/exception.h>

#include <windows.h>
#include <dxerr.h>
#pragma comment( lib, "dxerr.lib" )

template< typename T > inline void DIRECT_X_ADD_REF( T& x ) { if ( x ) { x->AddRef(); } }
template< typename T > inline void DIRECT_X_RELEASE( T& x ) { if ( x ) { x->Release(); } x = 0; }

#define DIRECT_X_FAIL_CHECK( x ) { HRESULT hresult = x; if ( FAILED( hresult ) ) { throw common::exception< HRESULT >( __FILE__, __LINE__, hresult ); } }


#endif // DIRECT_X_H
