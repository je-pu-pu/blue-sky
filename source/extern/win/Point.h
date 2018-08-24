//**********************************************************
//! copyright (c) JE all rights reserved
//!
//! \file		Point.h
//! \date		2003/06/18
//! \author		JE
//! \brief		Point
//**********************************************************

#ifndef WIN_POINT_H
#define WIN_POINT_H

#include <windows.h>

namespace win
{

// Point class
class Point
{
public:
	using unit = long;

private:
	POINT point_;

public:
	Point()
	{
		point_.x = 0;
		point_.y = 0;
	}

	Point( unit x, unit y )
	{
		point_.x = x;
		point_.y = y;
	}

    unit x() const { return point_.x; }
	unit y() const { return point_.y; }

	unit& x() { return point_.x; }
	unit& y() { return point_.y; }

	Point operator + ( const Point& p ) const { return Point( x() + p.x(), y() + p.y() ); }
	Point operator - ( const Point& p ) const { return Point( x() - p.x(), y() - p.y() ); }

	Point& operator += ( const Point& p ) { point_.x += p.x(); point_.y += p.y(); return *this; }
	Point& operator -= ( const Point& p ) { point_.x -= p.x(); point_.y -= p.y(); return *this; }
	
	operator POINT & () { return point_; }
	operator const POINT & () const { return point_; }

	POINT& get_point() { return point_; }
	const POINT& get_point() const { return point_; }

}; // class Point

} // namespace win

#endif // WIN_POINT_H
