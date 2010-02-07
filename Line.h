#ifndef ART_LINE_H
#define ART_LINE_H

#include "Type.h"

namespace art
{

/**
 * 
 */
class Line
{
private:
	Index _from;
	Index _to;

public:
	Line( Index from = 0, Index to = 0 )
		: _from( from )
		, _to( to )
	{ }

	~Line() { }

	Index& from() { return _from; }
	Index& to() { return _to; }

	const Index& from() const { return _from; }
	const Index& to() const { return _to; }
};

}

#endif // ART_MODEL_H