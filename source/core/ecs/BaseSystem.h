#pragma once

#include <core/type.h>

namespace core::ecs
{

/**
 * �V�X�e���̊��N���X
 *
 */
class BaseSystem
{
public:

public:
	BaseSystem() = default;
	virtual ~BaseSystem() = default;

	virtual void update() = 0;

}; // class System

} // namespace core::ecs
