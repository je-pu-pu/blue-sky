#pragma once

namespace core::graphics
{

class EffectPass
{
public:
	virtual ~EffectPass() { }
	virtual void apply() const = 0;

}; // class EffectPass

} // namespace core::graphics
