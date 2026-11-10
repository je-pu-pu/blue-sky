#pragma once

#include <blue_sky/type.h>
#include <unordered_map>
#include <functional>

namespace blue_sky
{

class Scene;
class GameMain;

/**
 * 複数の Scene を管理する Manager
 *
 */
/// シーンの作り方の一覧（組み立ての根の GameMain が持つ。以前はシングルトン）
class SceneManager
{
public:
	using SceneGenerator = std::function< Scene* ( GameMain& ) >;	///< 組み立ての根（GameMain）を受け取ってシーンを作る
	using SceneGeneratorMap = std::unordered_map< string_t, SceneGenerator >;


private:
	SceneGeneratorMap scene_generator_map_;

public:
	SceneManager() { }
	~SceneManager() { }

	/**
	 * 全てのシーンを登録する
	 */
	void register_all_scene();

	/**
	 * 指定した名前のシーンを登録する
	 */
	template< typename SceneType >
	void register_scene( const string_t& scene_name = SceneType::name, SceneGenerator generator = [] ( GameMain& game_main ) { return new SceneType( game_main ); } )
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
	Scene* generate_scene( const string_t& scene_name, GameMain& game_main ) const
	{
		auto i = scene_generator_map_.find( scene_name );

		if ( i == scene_generator_map_.end() )
		{
			return nullptr;
		}

		return i->second( game_main );
	}


	const SceneGeneratorMap& get_scene_generator_map() const { return scene_generator_map_; }
};

} // namespace blue_sky
