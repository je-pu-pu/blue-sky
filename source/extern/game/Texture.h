#ifndef GAME_TEXTURE_H
#define GAME_TEXTURE_H

#include <type/type.h>

namespace game
{

/**
 * 抽象テクスチャクラス
 *
 */
class Texture
{
public:
	Texture() { }
	virtual ~Texture() { }

	virtual uint_t get_width() const = 0;
	virtual uint_t get_height() const = 0;

	virtual uint_t get_multi_sample_count() const = 0;
	virtual uint_t get_multi_sample_quality() const = 0;

	virtual void bind_to_ds( uint_t slot ) const = 0;
	virtual void bind_to_ps( uint_t slot ) const = 0;

}; // Texture

class Color
{

};

class RenderTargetTexture : public Texture
{
public:
	// virtual void activate() = 0;
	// virtual void clear( const Color& ) = 0;

	virtual ~RenderTargetTexture() { }
};

class BackBufferTexture : public RenderTargetTexture
{
public:
	virtual ~BackBufferTexture() { }
};

} // namespace game

#endif // GAME_TEXTURE_H
