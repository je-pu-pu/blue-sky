#ifndef COMMON_SAFE_PTR_H
#define COMMON_SAFE_PTR_H

#include <cassert>

namespace common
{

/**
 * コピーできず、デストラクタで自動的に delete されないスマートポインタ
 * 
 */
template< typename T >
class [[deprecated("please use std::unique_ptr")]] safe_ptr
{
private:
	T* ptr_;

	safe_ptr( const safe_ptr& ) = delete;
	safe_ptr& operator = ( const safe_ptr& ) = delete;

public:
	safe_ptr() : ptr_( 0 ) { }
	explicit safe_ptr( T* ptr ) : ptr_( ptr ) { }

	safe_ptr& operator = ( T* ptr ) { assert( ! ptr_ ); ptr_ = ptr; return *this; }

	T* operator -> () { return ptr_; }
	const T* operator -> () const { return ptr_; }
	
	// operator T* () { return ptr_; }
	// operator const T* () const { return ptr_; }

	T* get() { return ptr_; }
	const T* get() const { return ptr_; }

	operator bool () const { return ptr_ != 0; }

	void release()
	{
		if ( ptr_ )
		{
			delete ptr_;
			ptr_ = 0;
		}
	}
};

} // namespace common

#endif // COMMON_SAFE_PTR_H
