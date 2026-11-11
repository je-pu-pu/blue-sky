#pragma once

#include <type/type.h>   // uint_t

#include <atomic>
#include <cstddef>       // std::size_t
#include <type_traits>
#include <vector>

namespace core::ecs
{

class Component;

using EntityId = uint_t;

// 部品・System の型の番号。0 から振る通し番号（get_component_type_id・get_system_type_id）。
// 以前は typeid( T ).hash_code() で、MSVC では呼ぶたびに装飾名を走査して計算していた（衝突もし得た）
using ComponentTypeId = std::size_t;
using SystemTypeId = std::size_t;

using ComponentList = std::vector< Component* >;

namespace detail
{

/// 次の部品の型の番号（型ごとに一度だけ呼ばれる。別のスレッドから初めて引かれても重ならないよう atomic）
inline ComponentTypeId next_component_type_id()
{
	static std::atomic< ComponentTypeId > next{ 0 };
	return next.fetch_add( 1 );
}

/// 次の System の型の番号
inline SystemTypeId next_system_type_id()
{
	static std::atomic< SystemTypeId > next{ 0 };
	return next.fetch_add( 1 );
}

template< typename ComponentType >
ComponentTypeId component_type_id_of()
{
	static const ComponentTypeId id = next_component_type_id();
	return id;
}

template< typename SystemType >
SystemTypeId system_type_id_of()
{
	static const SystemTypeId id = next_system_type_id();
	return id;
}

} // namespace detail

/**
 * 部品の型の番号を返す
 *
 * 型ごとに初めて引いたときに一度だけ振る（引いた順なので、起動ごとに番号が変わり得る。保存には使わない）。
 * 全部が 1 つの exe に静的にリンクされる（DLL をまたがない）ので、型ごとに 1 つに決まる。
 * const の有無は区別しない（以前の typeid と同じ）
 */
template< typename ComponentType >
ComponentTypeId get_component_type_id()
{
	return detail::component_type_id_of< std::remove_cv_t< ComponentType > >();
}

/// System の型の番号を返す（get_component_type_id と同じ決まり）
template< typename SystemType >
SystemTypeId get_system_type_id()
{
	return detail::system_type_id_of< std::remove_cv_t< SystemType > >();
}

} // namespace core::ecs
