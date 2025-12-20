#pragma once

#include <blue_sky/type.h>
#include <common/Singleton.h>
#include <unordered_map>
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

	/**
	 * シーンを自動登録するためのヘルパークラス
	 * 
	 * 各シーンのソースファイル内で static なインスタンスを生成することで、シーンを自動登録できるようにする。
	 * ただし、現状はコードの最適化により、インスタンス生成が削除されてしまうため、使用しない。代わりに SceneManager::register_all_scene() 内で手動登録している。
	 * 
	 * SceneRegister::SceneRegister< MyScene > scene_register; // MyScene::name と new MyScene() を使って登録する
	 * SceneRegister::SceneRegister< MyScene > scene_register( "name", [] { new MyScene( 1, 2.f, "3" ) } ) ; // 指定した名前と関数を使って登録する
	 */
	template < typename SceneType >
	class SceneRegister
	{
	public:
		SceneRegister( const string_t& scene_name = SceneType::name, SceneGenerator generator = [] { return new SceneType(); } )
		{
			SceneManager::get_instance()->register_scene< SceneType >( scene_name, generator );
		}
	};

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
	void register_scene( const string_t& scene_name = SceneType::name, SceneGenerator generator = [] { return new SceneType(); } )
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


	const SceneGeneratorMap& get_scene_generator_map() const { return scene_generator_map_; }
};

} // namespace blue_sky
