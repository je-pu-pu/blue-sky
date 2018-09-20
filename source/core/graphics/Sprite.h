#pragma once

#include <win/Rect.h>
#include <core/type.h>

namespace win
{
	class Rect;
	class Point;
}

namespace game
{
	class Texture;
}

namespace core::graphics
{

/**
 * Sprite
 *
 */
class Sprite
{
public:
	using Texture		= game::Texture;
	
	using Rect			= win::Rect;
	using Point			= win::Point;

protected:
	static const Color&	White;

	virtual void draw( const Rect*, const Texture*, const Rect*, const Color* ) = 0;

public:
	explicit Sprite() { }
	virtual ~Sprite() { }

	virtual void set_transform( const Matrix& ) = 0;
	virtual void set_ortho_offset( float_t ortho_offset ) = 0;

	virtual void begin() = 0;

	void draw( const Point& dp, const Texture* t, const Rect& sr, const Color& c = White )
	{
		Rect dr = Rect::Size( dp.x(), dp.y(), sr.width(), sr.height() );
		draw( & dr, t, & sr, & c );
	}

	void draw( const Rect& dr, const Texture* t, const Rect& sr, const Color& c = White )
	{
		draw( & dr, t, & sr, & c );
	}

	void draw( const Rect& dr, const Texture* t, const Color& c = White )
	{
		draw( & dr, t, 0, & c );
	}

	void draw( const Texture* t, const Rect& sr, const Color& c = White )
	{
		draw( 0, t, & sr, & c );
	}

	void draw( const Texture* t, const Color& c = White )
	{
		draw( 0, t, 0, & c );
	}

	virtual void end() = 0;

}; // class Sprite

} // namespace core::graphics
