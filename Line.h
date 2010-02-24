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
	ID _from;
	ID _to;

public:
	Line( ID from = 0, ID to = 0 )
		: _from( from )
		, _to( to )
	{ }

	~Line() { }

	ID& from() { return _from; }
	ID& to() { return _to; }

	const ID& from() const { return _from; }
	const ID& to() const { return _to; }
};

}

#endif // ART_MODEL_H