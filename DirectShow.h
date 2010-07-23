#ifndef DIRECT_SHOW_H
#define DIRECT_SHOW_H

#include <dshow.h>

/**
 * DirectShow ƒNƒ‰ƒX
 *
 */
class DirectShow
{
public:
	

private:
	IGraphBuilder*			graph_builder_;
	ICaptureGraphBuilder2*	capture_graph_builder_;
	IBaseFilter*			base_fildter_;
	IMediaControl*			media_control_;
	

public:
	DirectShow();
	~DirectShow();

}; // class DirectShow

#endif // DIRECT_SHOW_H