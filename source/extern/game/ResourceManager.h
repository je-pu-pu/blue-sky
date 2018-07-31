#pragma once

#include <type/type.h>
#include <unordered_map>

namespace game
{

/**
 * ���\�[�X�Ǘ��e���v���[�g�N���X
 *
 * ���̃N���X�͕����̃��\�[�X���Ǘ�����
 *
 * �S�Ẵ��\�[�X�͂��̃N���X�̃f�X�g���N�^�ɂ�胁������������
 *
 * ���̃N���X�̓��\�[�X�ɖ��O��t���ĊǗ����A���O�Ń��\�[�X���������擾�ł���
 * ( ����̓X�N���v�g����̃A�N�Z�X��e�Ղɂ��� )
 */
template< typename ResourceType >
class ResourceManager
{
public:
	typedef ResourceType Resource;
	typedef std::vector< std::unique_ptr< Resource > > ResourceList;
	typedef std::unordered_map< string_t, Resource* > ResourceMap;

private:
	ResourceList resource_list_;
	ResourceMap resource_map_;

public:
	ResourceManager()
	{

	}

	virtual ~ResourceManager()
	{

	}

	/**
	 * �w�肵���^�̃��\�[�X���쐬����
	 *
	 */
	template< typename Type = Resource, typename ... Args >
	Type* create( Args ... args )
	{
		Type* resource = new Type( args ... );
		resource_list_.emplace_back( resource );

		return resource;
	}

	/**
	 * �w�肵���^�̃��\�[�X���쐬�����O��t����
	 *
	 * @param name �쐬�������\�[�X�ɕt���閼�O
	 */
	template< typename Type = Resource, typename ... Args >
	Type* create_named( const string_t& name, Args ... args )
	{
		Type* resource = create< Type >( args ... );
		resource_map_[ name ] = resource;

		return resource;
	}

	/**
	 * �O���ō쐬�������\�[�X��ǉ��� ResourceManager �ŊǗ�����
	 *
	 * @param resource ���\�[�X
	 */
	void add( ResourceType* resource )
	{
		resource_list_.emplace_back( resource );
	}

	/**
	 * �O���ō쐬�������\�[�X�𖼑O��t���Ēǉ��� ResourceManager �ŊǗ�����
	 *
	 * @param name ���\�[�X�ɕt���閼�O
	 * @param resource ���\�[�X
	 */
	void add_named( const string_t& name, ResourceType* resource )
	{
		add( resource );
		resource_map_[ name ] = resource;
	}

	/**
	 * ���O�Ń��\�[�X���������擾����
	 *
	 * @param name ���\�[�X�ɕt�������O
	 * @return ���\�[�X�����������ꍇ�̓��\�[�X���A������Ȃ������ꍇ�� nullptr ��Ԃ�
	 */
	template< typename Type = ResourceType >
	Type* get( const string_t& name )
	{
		auto i = resource_map_.find( name );

		if ( i == resource_map_.end() )
		{
			return nullptr;
		}

		return static_cast< Type* >( i->second );
	}

	/**
	 * �S�Ẵ��\�[�X���폜����
	 *
	 */
	bool_t clear_all()
	{
		resource_list_.clear();
		resource_map_.clear();
	}

	/**
	 * �f�o�b�O�̂��߂Ƀ��\�[�X�̈ꗗ��W���o�͂ɏo�͂���
	 *
	 */
	void debug_print_resources() const
	{
		std::cout << "list : " << resource_list_.size() << ", named : " << resource_map_.size() << std::endl;

		for ( const auto& r : resource_map_ )
		{
			std::cout << "\t" << r.first << " : " << r.second << std::endl;
		}
	}
};

} // namespace game
