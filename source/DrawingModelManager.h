#pragma once

#include <core/type.h>
#include <map>

namespace blue_sky
{

class DrawingModel;

/**
 * 複数の DrawingModel を管理する Manager
 *
 */
class DrawingModelManager
{
public:
	typedef std::map< std::string, DrawingModel* > DrawingModelList;

private:
	DrawingModelList drawing_model_list_;

public:
	DrawingModelManager();
	~DrawingModelManager();

	void clear();

	DrawingModel* load( const char_t* );
	DrawingModel* get( const char_t* );
};

} // namespace blue_sky
