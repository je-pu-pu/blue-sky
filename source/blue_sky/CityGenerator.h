#pragma once

#include <blue_sky/type.h>
#include <unordered_set>

namespace core::graphics
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
 * 街の自動生成
 *
 */
class CityGenerator
{
public:
	using Model			= graphics::Model;
	using Mesh			= graphics::Mesh;
	using VertexGroup	= core::graphics::VertexGroup;

	/// 道路タイルの中心を指すコントロールポイント
	struct RoadNode
	{
		enum class Type
		{
			STRAIGHT,			///< 直線
			CROSS,				///< 十字路
			CURVE,				///< カーブ
			T_INTERSECTION,		///< T 字路
		};

		Type type;
		Vector position;
		RoadNode* front_node = nullptr;
		RoadNode* back_node  = nullptr;
		RoadNode* left_node  = nullptr;
		RoadNode* right_node = nullptr;

		RoadNode( Type t, const Vector& pos )
			: type( t )
			, position( pos )
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

	struct RoadControlPoint
	{
		enum class Type
		{
			FRONT,
			LEFT,
			RIGHT,
			BACK
		};

		Type type = Type::FRONT;
		Vector position = Vector::Zero;
		Vector front = Vector::Forward;
		RoadNode* node = nullptr;

		RoadControlPoint( Type t, const Vector& p, const Vector& f, RoadNode* n )
			: type( t )
			, position( p )
			, front( f )
			, node( n )
		{ }

		bool is_crossable() const
		{			
			auto straight_road_count = 0;

			for ( auto* n = node; n && n->type == RoadNode::Type::STRAIGHT; n = n->back_node )
			{
				straight_road_count++;

				if ( straight_road_count == CityGenerator::get_required_straight_road_count() )
				{
					return true;
				}
			}

			return false;
		}
	};

	using RoadNodeList = std::vector< RoadNode >;
	using RoadControlPointList = std::list< RoadControlPoint >;

private:
	Model* model_;

	RoadNodeList road_node_list_;
	RoadControlPointList road_control_point_list_;

protected:
	static constexpr float_t get_road_width() { return 8.f; }
	static constexpr float_t get_road_depth() { return 8.f; }
	static constexpr int_t get_required_straight_road_count() { return 5; }

	void extend_road( const Vector&, const Vector&, const Vector&, float_t, int_t, int_t );
	void generate_road_mesh();
	void generate_road_mesh( const RoadNode& );

public:
	CityGenerator();

	void step();

	Model* get_model() { return model_; }
};

} // namespace blue_sky
