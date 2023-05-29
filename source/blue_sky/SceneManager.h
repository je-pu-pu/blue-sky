#pragma once

#include <common/Singleton.h>
#include <functional>

namespace blue_sky
{

class Scene;

/**
 * •¡”‚Ì Scene ‚ğŠÇ—‚·‚é Manager
 *
 */
class SceneManager : public common::Singleton< SceneManager >
{
public:
	using SceneGenerator = std::function< Scene* () >;
	using SceneGeneratorMap = std::unordered_map< string_t, SceneGenerator >;

	template< typename SceneType >
	class RegisterScene
	{
	public:
		RegisterScene()
		{
			SceneManager::get_instance()->register_scene< SceneType >();
		}
	};

private:
	SceneGeneratorMap scene_generator_map_;

public:
	SceneManager() { }
	~SceneManager() { }


	template< typename SceneType >
	void register_scene()
	{
		scene_generator_map_[ SceneType::name ] = [] { return new SceneType(); };
	}


};

} // namespace blue_sky
