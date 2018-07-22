#pragma once

#include <vector>

namespace core::graphics
{

class EffectPass;

class EffectTechnique
{
public:
	typedef std::vector< EffectPass* >	PassList;

public:
	virtual const PassList& get_pass_list() const = 0;
};

} // namespace core::graphics
