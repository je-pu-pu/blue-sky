#ifndef DIRECT_X_H
#define DIRECT_X_H

#include <dxerr9.h>

#define DIRECT_X_FAIL_CHECK( x ) { HRESULT hresult = x; if ( FAILED( hresult ) ) { throw common::exception< HRESULT >( __FILE__, __LINE__, hresult ); } }

#endif // DIRECT_X_H
