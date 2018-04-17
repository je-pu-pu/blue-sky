#ifndef BLUE_SKY_DRAWING_MODEL_MANAGER_H
#define BLUE_SKY_DRAWING_MODEL_MANAGER_H

#include <core/type.h>
#include <map>

namespace blue_sky
{

class DrawingModel;

/**
 * ï°êîÇÃ DrawingModel Çä«óùÇ∑ÇÈ Manager
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

	DrawingModelList& active_object_list() { return drawing_model_list_; }
	const DrawingModelList& active_object_list() const { return drawing_model_list_; }
};

} // namespace blue_sky

#endif // BLUE_SKY_DRAWING_MODEL_MANAGER_H