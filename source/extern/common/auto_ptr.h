#ifndef COMMON_AUTO_PTR_H
#define COMMON_AUTO_PTR_H

#include <cassert>

namespace common
{

template< typename T >
class [[deprecated("please use std::unique_ptr")]] auto_ptr
{
private:
	T* ptr_;

	auto_ptr( const auto_ptr& ) = delete;
	auto_ptr& operator = ( const auto_ptr& ) = delete;

public:
	inline auto_ptr() : ptr_( 0 ) { }
	inline explicit auto_ptr( T* ptr ) : ptr_( ptr ) { }
	inline ~auto_ptr() { release(); }

	inline auto_ptr& operator = ( T* ptr ) { assert( ! ptr_ ); ptr_ = ptr; return *this; }

	inline T* operator -> () { return ptr_; }
	inline const T* operator -> () const { return ptr_; }
	
	// inline operator T* () { return ptr_; }
	// inline operator const T* () const { return ptr_; }

	inline T* get() { return ptr_; }
	inline T* get() const { return ptr_; }

	inline operator bool () const { return ptr_ != 0; }

	inline void release()
	{
		if ( ptr_ )
		{
			delete ptr_;
			ptr_ = 0;
		}
	}
};

} // namespace common

#endif // COMMON_AUTO_PTR_H
