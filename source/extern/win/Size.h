//**********************************************************
//! copyright (c) JE all rights reserved
//!
//! \file		Size.h
//! \date		2013/07/19
//! \author		JE
//! \brief		Size
//**********************************************************

#ifndef WIN_SIZE_H
#define WIN_SIZE_H

#include <windows.h>

namespace win
{

/**
 * ïùÇ∆çÇÇ≥ÇéùÇ¬ÉTÉCÉY
 *
 */
class Size
{
public:
	typedef long unit;

private:
	unit width_;
	unit height_;

public:
	Size()
	{
		width_ = 0;
		height_ = 0;
	}

	Size( unit w, unit h )
	{
		width_ = w;
		height_ = h;
	}

	unit& width() { return width_; }
	unit& height() { return height_; }

    unit width() const { return width_; }
	unit height() const { return height_; }

	Size operator + ( const Size& s ) const { return Size( width() + s.width(), height() + s.height() ); }
	Size operator - ( const Size& s ) const { return Size( width() - s.width(), height() - s.height() ); }

	Size & operator += ( const Size & s ) { width() += s.width(); height() += s.height(); return *this; }
	Size & operator -= ( const Size & s ) { width() -= s.width(); height() -= s.height(); return *this; }
	
	operator SIZE () const { SIZE s = { width(), height() }; return s; }

}; // class Size

} // namespace win

#endif // WIN_SIZE_H
