#pragma once

#include <type/type.h>

namespace core::graphics
{
	class Texture;
	class ConstantBuffer;

/**
 * 抽象シャドウマップ
 *
 */
class ShadowMap
{
public:
	static const int MaxCascadeLevels = 4;

	using Vector	= Vector;
	using Texture	= Texture;

private:

public:
	ShadowMap() { }
	virtual ~ShadowMap() { }

	virtual void ready_to_render_shadow_map() = 0;
	virtual void ready_to_render_shadow_map_with_cascade_level( uint_t ) = 0;

	virtual void ready_to_render_scene() = 0;
	virtual void bind_to_render_scene() const = 0;

	virtual int get_cascade_levels() const = 0;
	
	virtual void set_light_position( const Vector& ) = 0;
	virtual void set_eye_position( const Vector& ) = 0;

	virtual const ConstantBuffer* get_constant_buffer() const = 0;

	virtual Texture* get_texture() = 0;
	virtual const Texture* get_texture() const = 0;

}; // class ShadowMap

} // core::graphics
