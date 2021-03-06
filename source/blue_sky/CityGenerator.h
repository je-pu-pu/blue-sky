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

	// 道路タイル
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

		bool is_end = false;	///< このノードが終端かどうか？ ( 他の道路にぶつかって止まったかどうか )

		Vector start_front;		///< 開始地点での前方単位ベクトル
		Vector start_right;		///< 開始地点での右方単位ベクトル

		Vector end_front;		///< 終了地点での前方単位ベクトル
		Vector end_right;		///< 終了地点での右方単位ベクトル

		Vector back_left_pos;
		Vector back_right_pos;
		Vector front_left_pos;
		Vector front_right_pos;

		RoadNode( Type t, const Vector& pos )
			: type( t )
			, position( pos )
		{
			update_vertex_pos();
		}

		~RoadNode()
		{
			unlink();
		}

		void unlink()
		{
			if ( front_node )
			{
				front_node->unlink( this );
			}

			if ( back_node )
			{
				back_node->unlink( this );
			}

			if ( left_node )
			{
				left_node->unlink( this );
			}

			if ( right_node )
			{
				right_node->unlink( this );
			}

			unlink( front_node );
			unlink( back_node );
			unlink( left_node );
			unlink( right_node );
		}

		/**
		 * 指定したノードとのリンクを解除する
		 */
		void unlink( RoadNode* node )
		{
			if ( front_node == node )
			{
				front_node = nullptr;
			}

			if ( back_node == node )
			{
				back_node = nullptr;
			}

			if ( left_node == node )
			{
				left_node = nullptr;
			}

			if ( right_node == node )
			{
				right_node = nullptr;
			}
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

		/**
		 * 指定したノードと衝突しているかを返す
		 *
		 * @parma node ノード
		 */
		bool is_collition_with( const RoadNode& )
		{
			return true;
		}

		void update_vertex_pos();
	};

	/**
	 * 伸びて行く道路の先頭を指すコントロールポイント
	 */
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

		/**
		 * 現在伸びている方向から見て後ろに数えて行き、必要な直線タイル分伸びていれば、交差点になれると返す
		 */
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

		Vector front_left_pos() const { return node->front_left_pos; }
		Vector front_right_pos() const { return node->front_right_pos; }
	};

	using RoadNodeList = std::vector< std::unique_ptr< RoadNode > >;
	using RoadControlPointList = std::list< RoadControlPoint >;

private:
	Model* model_; // 街のモデル
	Model* debug_model_; // デバッグ用のモデル

	RoadNodeList road_node_list_;
	RoadControlPointList road_control_point_list_;

protected:
	/// 道 1 タイル分の幅 ( m )
	static constexpr float_t get_road_width() { return 8.f; }

	// 道 1 タイル分の奥行 ( m )
	static constexpr float_t get_road_depth() { return 8.f; }

	// 交差点ではなく、まっすぐな道が何タイル続くか
	static constexpr int_t get_required_straight_road_count() { return 5; }

	void format_crossroad();

	void extend_road( const Vector&, const Vector&, const Vector&, float_t, int_t, int_t );
	void generate_road_mesh();
	void generate_road_mesh( const RoadNode* );
	
	void generate_debug_mesh();
	void generate_debug_road_control_point_mesh( const RoadControlPoint& ) const;

	bool check_collision( RoadControlPoint& ) const;

	VertexGroup* get_vertex_group_by_road_node( const RoadNode* );

public:
	CityGenerator();

	void step();

	Model* get_model() { return model_; }

	Model* get_debug_model() { return debug_model_; }
};

} // namespace blue_sky
