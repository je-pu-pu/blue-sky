#include "Mesh.h"

namespace blue_sky::graphics
{

/**
 * �V�������_�O���[�v���쐬����
 *
 * @return �쐬���ꂽ���_�O���[�v
 */
Mesh::VertexGroup* Mesh::create_vertex_group()
{
	vertex_group_list_.push_back( std::make_unique< VertexGroup >() );

	return vertex_group_list_.back().get();
}

/**
 * ���_�O���[�v�̈ꗗ�̒�����w�肵���C���f�b�N�X�̒��_�O���[�v���擾����
 *
 * @param index ���_�O���[�v�̃C���f�b�N�X
 * @return ���_�O���[�v or 0
 */
Mesh::VertexGroup* Mesh::get_vertex_group_at( uint_t index )
{
	while ( index >= vertex_group_list_.size() )
	{
		create_vertex_group();
	}

	return vertex_group_list_[ index ].get();
}

/**
 * �œK������
 *
 *
 */
void Mesh::optimize()
{
	
}

/**
 * �V�X�e����������̒��_�����N���A����
 *
 */
void Mesh::clear_vertex_list()
{
	vertex_list_.clear();
}

/**
 * �V�X�e����������̃E�G�C�g�����N���A����
 *
 */
void Mesh::clear_vertex_weight_list()
{
	vertex_weight_list_.clear();
}

/**
 * �V�X�e����������̃C���f�b�N�X�����N���A����
 *
 */
void Mesh::clear_vertex_group_list()
{
	vertex_group_list_.clear();
}

/**
 * ���_�� Z �l�𔽓]����
 *
 */
void Mesh::invert_vertex_z()
{
	for ( auto& v : vertex_list_ )
	{
		v.Position.z() = -v.Position.z();
		v.Normal.z() = -v.Normal.z();
	}
}

/**
 * �|���S���̗��\�𔽓]����
 *
 */
void Mesh::flip_polygon()
{
	for ( auto& vertex_group : vertex_group_list_ )
	{
		vertex_group->flip_polygon();
	}
}

} // namespace blue_sky::graphics
