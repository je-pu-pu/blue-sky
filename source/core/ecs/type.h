#pragma once

#include <core/type.h>
#include <vector>
#include <cstddef>

namespace core::ecs
{

class Component;

using EntityId = uint_t;
// 型 ID は Component::get_type_id() が返す std::size_t（x64 で 64bit）と一致させる。
// uint_t(32bit) にすると x64 で上位ビットが切り捨てられ、System の型一致判定
// (has_component_type) が壊れて on_add_component 経由の自動登録が全滅する。
using ComponentTypeId = std::size_t;
using ComponentList = std::vector< Component* >;

} // namespace core::ecs
