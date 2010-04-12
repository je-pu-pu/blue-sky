#ifndef ART_FACE_H
#define ART_FACE_H

#include "Color.h"
#include "Type.h"
#include <vector>

namespace art
{

/**
 * 面
 *
 * 
 */
class Face
{
public:
	typedef std::vector<ID> IDList;

private:
	IDList id_list_;		///< ID の一覧
	Color color_;			///< 色

public:
	IDList& id_list() { return id_list_; }
	const IDList& id_list() const { return id_list_; }

	Color& color() { return color_; }
	const Color& color() const { return color_; }
};

}

#endif // ART_FACE_H