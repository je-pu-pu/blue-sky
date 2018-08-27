#pragma once

#include <blue_sky/type.h>

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
 * 街の自動生成
 *
 */
class CityGenerator
{
public:
	using Model			= graphics::Model;
	using Mesh			= graphics::Mesh;
	using VertexGroup	= game::VertexGroup;

	/// 道路タイルの中心を指すコントロールポイント

	struct RoadControlPoint
	{
		enum class Type
		{
			STRAIGHT,			///< 直線
			CROSS,				///< 十字路
			CURVE,				///< カーブ
			T_INTERSECTION,		///< T 字路
		};

		Vector position;
		float_t direction;
	};

	using RoadControlPointList = std::vector< RoadControlPoint >;

private:
	Model* model_;

	RoadControlPointList road_control_point_list_;

	Vector control_point_;
	// float_t direction_ = 0.f;

protected:
	void extend_road( const Vector&, const Vector&, const Vector&, float_t, int_t, int_t );
	void generate_road_mesh();

public:
	CityGenerator();

	void step();

	Model* get_model() { return model_; }
};

} // namespace blue_sky
