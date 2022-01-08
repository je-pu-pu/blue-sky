#pragma once

#include <core/type.h>

namespace core::ecs
{

/**
 * システムの基底クラス
 *
 */
class System
{
public:

public:
	System() = default;
	virtual ~System() = default;

	virtual void update() = 0;

}; // class System

} // namespace core::ecs
