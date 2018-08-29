#pragma once

#include "Scene.h"
#include <blue_sky/CityGenerator.h>
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
	CityGenerator city_generator_;

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