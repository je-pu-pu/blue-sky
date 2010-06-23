#ifndef DIRECT_X_H
#define DIRECT_X_H

#include <dxerr9.h>

#define DIRECT_X_FAIL_CHECK( x ) { HRESULT hr = x; if ( FAILED( hr ) ) { std::string m = std::string( DXGetErrorString9( hr ) ) + " : " + DXGetErrorDescription9( hr ); COMMON_THROW_EXCEPTION_MESSAGE( m ); } }

#endif // DIRECT_X_H
