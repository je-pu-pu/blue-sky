#pragma once

namespace common
{

/**
 * シングルトン共通基底クラス
 */
template< typename T >
class Singleton
{
protected:
    Singleton() { }

public:
    static T* get_instance() { static T instance; return & instance; }

    Singleton( const Singleton& ) = delete;
    Singleton& operator = ( const Singleton& ) = delete;
    Singleton( Singleton&& ) = delete;
    Singleton& operator = ( Singleton&& ) = delete;
};  

} // namespace common
