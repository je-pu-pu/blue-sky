#pragma once

#include <type/type.h>
#include <vector>

namespace core::ecs
{

class Component;

using EntityId = uint_t;
using ComponentTypeId = uint_t;
using ComponentList = std::vector< Component* >;

} // namespace core::ecs
