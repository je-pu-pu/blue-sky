#pragma once

#include "BaseSystem.h"
#include <vector>

namespace core::ecs
{

/**
 * �V�X�e���̊��N���X
 *
 */
template< typename ... ComponentTypes >
class System : public BaseSystem
{
public:
	using ComponentTuple = std::tuple< std::add_pointer_t< ComponentTypes > ... >;
	using ComponentList = std::vector< ComponentTuple >;
	
private:
	ComponentList component_list_;

protected:
	ComponentList& get_component_list() { return component_list_; }
	const ComponentList& get_component_list() const { return component_list_; }

public:
	System() = default;
	virtual ~System() = default;

}; // class System

} // namespace core::ecs
