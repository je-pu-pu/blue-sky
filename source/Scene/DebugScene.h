#pragma once

#include "Scene.h"
#include <memory>

namespace blue_sky
{

class Camera;

/**
 * デバッグ用シーン
 *
 */
class DebugScene : public Scene
{
public:

protected:

	/// @todo Scenegraph に移動する
	std::unique_ptr< Camera > camera_;

public:
	explicit DebugScene( const GameMain* );
	~DebugScene() override;

	void update() override;
	void render() override;

};

} // namespace blue_sky