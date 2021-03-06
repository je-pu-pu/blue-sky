#pragma once

#include <common/exception.h>

template< typename T > inline void DIRECT_X_ADD_REF( T& x ) { if ( x ) { x->AddRef(); } }
template< typename T > inline void DIRECT_X_RELEASE( T& x ) { if ( x ) { x->Release(); } x = nullptr; }

#define DIRECT_X_FAIL_CHECK( x ) { HRESULT hresult = x; if ( FAILED( hresult ) ) { throw common::exception< HRESULT >( __FILE__, __LINE__, hresult ); } }

template< typename T >
class com_ptr
{
private:
	T*		ptr_;

	inline com_ptr& operator = ( T* ptr )
	{
		DIREXT_X_RELEASE( ptr_ );
		ptr_ = ptr;
		DIREXT_X_ADD_REF( ptr_ );

		return *this;
	}
	inline com_ptr& operator = ( const com_ptr& ptr )
	{
		DIRECT_X_RELEASE( ptr_ );
		ptr_ = ptr.ptr_;
		DIREXT_X_ADD_REF( ptr_ );

		return *this;
	}

public:
	explicit com_ptr()
		: ptr_( nullptr )
	{
		//
	}

	explicit com_ptr( T* ptr )
		: ptr_( ptr )
	{
		//
	}

	explicit com_ptr( const com_ptr& ptr )
		: ptr_( ptr.ptr_ )
	{
		DIRECT_X_ADD_REF( ptr_ );
	}

	~com_ptr()
	{
		DIRECT_X_RELEASE( ptr_ );
	}
	
	void reset( T* ptr = nullptr )
	{
		DIRECT_X_RELEASE( ptr_ );

		ptr_ = ptr;
	}

	inline T** operator & () { return & ptr_; }
	inline T* const * operator & () const { return & ptr_; }

	inline T* operator -> () { return ptr_; }
	inline const T* operator -> () const { return ptr_; }

	inline T* get() { return ptr_; }
	inline T* get() const { return ptr_; }
}; 
