//**********************************************************
//! copyright (c) JE all rights reserved
//!
//! \file		Version.h
//! \date		2012/07/15
//! \author		JE
//! \brief		Windows Version Utilities
//**********************************************************

#ifndef WIN_VERSION_H
#define WIN_VERSION_H

namespace win
{

class Version
{
private:

public:
	Version() { }
	~Version() { }

	bool log( const char* ) const;

}; // class Version

} // namespace win

#endif // WIN_VERSION_H