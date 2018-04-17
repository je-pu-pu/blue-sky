#include "DrawingModelManager.h"
#include "DrawingModel.h"

namespace blue_sky
{

DrawingModelManager::DrawingModelManager()
{
	
}

DrawingModelManager::~DrawingModelManager()
{
	clear();
}

void DrawingModelManager::clear()
{
	for ( DrawingModelList::iterator i = drawing_model_list_.begin(); i != drawing_model_list_.end(); ++i )
	{
		delete i->second;
	}

	drawing_model_list_.clear();
}

DrawingModel* DrawingModelManager::load( const char_t* name )
{
	DrawingModelList::const_iterator i = drawing_model_list_.find( name  );

	if ( i != drawing_model_list_.end() )
	{
		return i->second;
	}

	DrawingModel* drawing_model = new DrawingModel();
	drawing_model->load( name );
	
	drawing_model_list_[ name ] = drawing_model;

	return drawing_model;
}

DrawingModel* DrawingModelManager::get( const char_t* name )
{
	return drawing_model_list_[ name ];
}

}; // namespace blue_sky