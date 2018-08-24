//**********************************************************
//! copyright (c) JE all rights reserved
//!
//! \file		Rect.h
//! \date		2003/06/18
//! \author		JE
//! \brief		Rect
//**********************************************************

#ifndef WIN_RECT_H
#define WIN_RECT_H

#include <windows.h>
#include "Point.h"

namespace win
{

//! Rect class
class Rect
{
public:
	using unit = LONG;

private:
	RECT rect_;

public:
	Rect()
	{
		SetRect( & rect_, 0, 0, 0, 0 );
	}

	Rect( unit left, unit top, unit right, unit bottom )
	{ 
		SetRect( & rect_, left, top, right, bottom );
	}

	Rect( const RECT & r )
		: rect_( r )
	{
		
	}
	
	unit left() const { return rect_.left; }
	unit top() const { return rect_.top; }
	unit right() const { return rect_.right; }
	unit bottom() const { return rect_.bottom; }

	unit width() const { return rect_.right - rect_.left; }
	unit height() const { return rect_.bottom - rect_.top; }

	unit& left() { return rect_.left; }
	unit& top() { return rect_.top; }
	unit& right() { return rect_.right; }
	unit& bottom() { return rect_.bottom; }
	
	Rect operator + ( const Point& p ) const { return Rect( left() + p.x(), top() + p.y(), right() + p.x(), bottom() + p.y() ); }
	Rect operator - ( const Point& p ) const { return Rect( left() - p.x(), top() - p.y(), right() - p.x(), bottom() - p.y() ); }

	Rect& operator += ( const Point& p ) { left() += p.x(); right() += p.x(); top() += p.y(); bottom() += p.y(); return *this; }
	Rect& operator -= ( const Point& p ) { left() -= p.x(); right() -= p.x(); top() -= p.y(); bottom() -= p.y(); return *this; }

	Rect operator + ( const Rect& r ) const { return Rect( left() + r.left(), top() + r.top(), right() + r.right(), bottom() + r.bottom() ); }
	Rect operator - ( const Rect& r ) const { return Rect( left() - r.left(), top() - r.top(), right() - r.right(), bottom() - r.bottom() ); }

	bool in( const Point& p ) const { return p.x() >= left() && p.x() < right() && p.y() >= top() && p.y() < bottom(); }

	Point get_left_top() const { return Point( left(), top() ); }

	operator RECT& () { return rect_; }
	operator const RECT& () const { return rect_; }
	
	RECT& get_rect() { return rect_; }
	const RECT& get_rect() const { return rect_; }

	static Rect Size( unit left, unit top, unit width, unit height )
	{
		return Rect( left, top, left + width, top + height );
	}

}; // class Rect

} // namespace win

#endif // WIN_RECT_H

