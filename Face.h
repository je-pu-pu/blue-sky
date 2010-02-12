#ifndef ART_FACE_H
#define ART_FACE_H

#include "Type.h"
#include <vector>

namespace art
{

/**
 * 
 */
class Face
{
public:
	typedef std::vector<Index> IndexList;

private:
	IndexList index_list_;

public:
	IndexList& index_list() { return index_list_; }
	const IndexList& index_list() const { return index_list_; }

};

}

#endif // ART_FACE_H