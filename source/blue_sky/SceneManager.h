#pragma once

#include <common/Singleton.h>
#include <functional>

namespace blue_sky
{

class Scene;

/**
 * 複数の Scene を管理する Manager
 *
 */
class SceneManager : public common::Singleton< SceneManager >
{
public:
	using SceneGenerator = std::function< Scene* () >;
	using SceneGeneratorMap = std::unordered_map< string_t, SceneGenerator >;

	template < typename SceneType >
	class RegisterScene
	{
	public:
		RegisterScene( const char_t* scene_name = SceneType::name, SceneGenerator generator = [] { return new SceneType(); } )
		{
			SceneManager::get_instance()->register_scene( scene_name, generator );
		}
	};

private:
	SceneGeneratorMap scene_generator_map_;

public:
	SceneManager() { }
	~SceneManager() { }

	/**
	 * 指定した名前のシーンを登録する
	 */
	void register_scene( const string_t& scene_name, SceneGenerator generator )
	{
		scene_generator_map_[ scene_name ] = generator;
	}

	/**
	 * 指定した名前のシーンが登録済みかどうかを調べる
	 */
	bool is_scene_registered( const string_t& scene_name ) const
	{
		return scene_generator_map_.find( scene_name ) != scene_generator_map_.end();
	}

	/**
	 * 指定した名前のシーンを生成する
	 */
	Scene* generate_scene( const string_t& scene_name ) const
	{
		auto i = scene_generator_map_.find( scene_name );

		if ( i == scene_generator_map_.end() )
		{
			return nullptr;
		}

		return i->second();
	}

};

} // namespace blue_sky
