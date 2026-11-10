#pragma once

#include "BaseFileLoader.h"

#include <optional>
#include <string>

namespace blue_sky::graphics
{

namespace obj
{
	/// 面の頂点 1 つの番号（0 始まり）。UV・法線は書かれていないこともある
	struct FaceVertex
	{
		size_t					position = 0;
		std::optional< size_t >	tex_coord;
		std::optional< size_t >	normal;
	};

	/**
	 * 面の頂点 1 つ（"位置"・"位置/UV"・"位置//法線"・"位置/UV/法線"。番号は 1 始まり）を 0 始まりの番号にする
	 *
	 * @return 書き方が正しくない（数でない・0 以下・欄が多すぎる）ときは nullopt
	 */
	std::optional< FaceVertex > parse_face_vertex( const std::string& );
}

/**
 * OBJ File Loader
 *
 */
class ObjFileLoader : public BaseFileLoader
{
public:
	ObjFileLoader( GraphicsManager* graphics_manager, Model* model, const char_t* name )
		: BaseFileLoader( graphics_manager, model, name )
	{ }

	bool load( const char_t* ) override;

}; // class ObjFileLoader

} // namespace blue_sky::graphics
