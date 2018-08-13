#pragma once

#include "Scene.h"
#include <memory>

namespace blue_sky
{

class Camera;

/**
 * �f�o�b�O�p�V�[��
 *
 */
class DebugScene : public Scene
{
public:

protected:

	/// @todo Scenegraph �Ɉړ�����
	std::unique_ptr< Camera > camera_;

public:
	explicit DebugScene( const GameMain* );
	~DebugScene() override;

	void update() override;
	void render() override;

};

} // namespace blue_sky