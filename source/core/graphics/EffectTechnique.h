#pragma once

#include <vector>

namespace core::graphics
{

class EffectPass;

class EffectTechnique
{
public:
	using PassList = std::vector< EffectPass* >;

public:
	virtual ~EffectTechnique() { }
	virtual const PassList& get_pass_list() const = 0;
};

} // namespace core::graphics
