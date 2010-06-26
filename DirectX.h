#ifndef DIRECT_X_H
#define DIRECT_X_H

#include <dxerr9.h>

#define DIRECT_X_FAIL_CHECK( x ) { HRESULT hresult = x; if ( FAILED( hresult ) ) { std::string m = std::string( DXGetErrorString9( hresult ) ) + " : " + DXGetErrorDescription9( hresult ); COMMON_THROW_EXCEPTION_MESSAGE( m ); } }

#endif // DIRECT_X_H
