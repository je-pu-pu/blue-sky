#pragma once

#include "Scene.h"
#include <blue_sky/CityGenerator.h>
#include <memory>

namespace core::graphics
{
	class RenderTargetTexture;
}

namespace blue_sky
{

class Camera;

/**
 * �p�[�e�B�N���V�X�e���̃e�X�g�p�V�[��
 *
 */
class ParticleSystemTestScene : public Scene
{
public:

protected:


protected:

	/// @todo Scenegraph �Ɉړ�����
	std::unique_ptr< Camera > camera_;

public:
	explicit ParticleSystemTestScene( const GameMain* );
	~ParticleSystemTestScene() override;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return false; }
};

} // namespace blue_sky