#pragma once

#include <memory>
#include <vector>

namespace core::graphics
{

class EffectPass;

class EffectTechnique
{
public:
	using PassList = std::vector< std::unique_ptr< EffectPass > >;

public:
	virtual ~EffectTechnique() { }
	virtual const PassList& get_pass_list() const = 0;
};

} // namespace core::graphics
