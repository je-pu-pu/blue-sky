#ifndef DIRECT_X_H
#define DIRECT_X_H

#include <common/exception.h>

#include <windows.h>
#include <dxerr.h>
#pragma comment( lib, "dxerr.lib" )

#define DIRECT_X_RELEASE( x ) if ( x ) { x->Release(); x = 0; }
#define DIRECT_X_FAIL_CHECK( x ) { HRESULT hresult = x; if ( FAILED( hresult ) ) { throw common::exception< HRESULT >( __FILE__, __LINE__, hresult ); } }


#endif // DIRECT_X_H
