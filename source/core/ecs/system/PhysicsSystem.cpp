#include "PhysicsSystem.h"
#include <iostream>

namespace core::ecs
{

PhysicsSystem::PhysicsSystem()
{
	//
}

void PhysicsSystem::update()
{
	std::cout << "PhysicsSystem::update()" << std::endl;
}

} // namespace core::ecs
