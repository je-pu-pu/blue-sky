#pragma once

#include <common/Singleton.h>
#include <functional>

namespace blue_sky
{

class Scene;

/**
 * ������ Scene ���Ǘ����� Manager
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
	 * �w�肵�����O�̃V�[����o�^����
	 */
	void register_scene( const string_t& scene_name, SceneGenerator generator )
	{
		scene_generator_map_[ scene_name ] = generator;
	}

	/**
	 * �w�肵�����O�̃V�[�����o�^�ς݂��ǂ����𒲂ׂ�
	 */
	bool is_scene_registered( const string_t& scene_name ) const
	{
		return scene_generator_map_.find( scene_name ) != scene_generator_map_.end();
	}

	/**
	 * �w�肵�����O�̃V�[���𐶐�����
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
