#ifndef ART_MODEL_H
#define ART_MODEL_H

#include "LiveVertex.h"
#include "Line.h"
#include "Face.h"
#include <vector>

namespace art
{

/**
 * 
 */
class Model
{
public:
	typedef std::vector<LiveVertex> VertexList;
	typedef std::vector<Line> LineList;
	typedef std::vector<Face> FaceList;

private:
	VertexList vertex_list_;
	LineList line_list_;
	FaceList face_list_;

public:
	VertexList& vertex_list() { return vertex_list_; }
	LineList& line_list() { return line_list_; }
	FaceList& face_list() { return face_list_; }

	const VertexList& vertex_list() const { return vertex_list_; }
	const LineList& line_list() const { return line_list_; }
	const FaceList& face_list() const { return face_list_; }
};

}

#endif // ART_MODEL_H