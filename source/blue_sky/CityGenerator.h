#pragma once

#include <blue_sky/type.h>
#include <unordered_set>

namespace game
{
	class VertexGroup;
}

namespace blue_sky
{
	namespace graphics
	{
		class Model;
		class Mesh;
	}

/**
 * �X�̎�������
 *
 */
class CityGenerator
{
public:
	using Model			= graphics::Model;
	using Mesh			= graphics::Mesh;
	using VertexGroup	= game::VertexGroup;

	/// ���H�^�C���̒��S���w���R���g���[���|�C���g
	struct RoadNode
	{
		enum class Type
		{
			STRAIGHT,			///< ����
			CROSS,				///< �\���H
			CURVE,				///< �J�[�u
			T_INTERSECTION,		///< T ���H
		};

		Vector position;
		RoadNode* front_node = nullptr;
		RoadNode* back_node  = nullptr;
		RoadNode* left_node  = nullptr;
		RoadNode* right_node = nullptr;

		RoadNode( const Vector& pos )
			: position( pos )
		{

		}

		void link_front( RoadNode& node )
		{
			front_node = & node;
			node.back_node = this;
		}

		void unlink_front()
		{
			if ( front_node )
			{
				front_node->back_node = nullptr;
			}
			
			front_node = nullptr;
		}
	};

	using RoadNodeList = std::vector< RoadNode >;

private:
	Model* model_;

	RoadNodeList road_node_list_;

	Vector control_point_;
	Vector control_point_front_;
	// float_t direction_ = 0.f;

protected:
	void extend_road( const Vector&, const Vector&, const Vector&, float_t, int_t, int_t );
	void generate_road_mesh();
	void generate_road_mesh( const RoadNode& );

public:
	CityGenerator();

	void step();

	Model* get_model() { return model_; }
};

} // namespace blue_sky
