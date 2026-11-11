#include "pch.h"

#include <core/physics/PhysicsManager.h>
#include <core/physics/RigidBody.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

// ---- PhysicsManager の本編のための機能（以前の BulletPhysics・blue_sky::ActiveObjectPhysics から移したもの） ----
// どれも Bullet の世界を実際に作って確かめる

namespace
{

using core::physics::PhysicsManager;
using core::physics::RigidBody;
using core::physics::RigidBodyCreateInfo;
using core::physics::RigidBodyShapeType;
using core::math::Vector;
using core::math::Transform;
using Tag = std::optional< core::ecs::EntityId >;

Transform at( float x, float y, float z )
{
	Transform t;
	t.set_identity();
	t.set_position( Vector( x, y, z ) );
	return t;
}

/// 本編の物体と同じ作り方（形を共有・Bullet の既定の摩擦と振り分け）で、動かない箱を作る
RigidBody make_box( PhysicsManager& physics, const Transform& transform, const Vector& half, float mass = 0.f )
{
	RigidBodyCreateInfo info;
	info.shape_type = RigidBodyShapeType::Box;
	info.shape_size = half;
	info.mass = mass;
	info.transform = transform;
	info.offset = Transform::identity();
	info.friction = RigidBodyCreateInfo::BulletDefaultFriction;
	info.share_shape = true;
	info.use_default_collision_filter = true;

	return physics.create_rigid_body( info );
}

/// 一時的な OBJ ファイル（壊れるときに消す）
struct TemporaryObj
{
	std::filesystem::path path;

	explicit TemporaryObj( const std::string& name, const std::string& text )
		: path( std::filesystem::temp_directory_path() / name )
	{
		std::ofstream out( path );
		out << text;
	}

	~TemporaryObj()
	{
		std::error_code e;
		std::filesystem::remove( path, e );
	}
};

} // namespace

TEST( PhysicsManagerTest, 形を共有する剛体は同じ種類と大きさなら形を使い回し_clearで消える )
{
	PhysicsManager physics;

	{
		auto a = make_box( physics, at( 0, 0, 0 ), Vector( 1.f, 2.f, 3.f ), 1.f );
		auto b = make_box( physics, at( 0, 5, 0 ), Vector( 1.f, 2.f, 3.f ), 1.f );
		EXPECT_EQ( 1u, physics.get_owned_shape_count() );

		auto c = make_box( physics, at( 0, 10, 0 ), Vector( 1.f, 2.f, 4.f ), 1.f );
		EXPECT_EQ( 2u, physics.get_owned_shape_count() );

		// 種類が違えば別の形
		RigidBodyCreateInfo info;
		info.shape_type = RigidBodyShapeType::Cylinder;
		info.shape_size = Vector( 1.f, 2.f, 3.f );
		info.share_shape = true;
		auto d = physics.create_rigid_body( info );
		EXPECT_EQ( 3u, physics.get_owned_shape_count() );
		EXPECT_EQ( RigidBodyShapeType::Cylinder, d.get_shape_type() );

		// 共有しない剛体（ECS のテストシーン）は、形を自分で持つ
		auto e = physics.create_rigid_body( RigidBodyCreateInfo() );
		EXPECT_EQ( 3u, physics.get_owned_shape_count() );
		EXPECT_EQ( RigidBodyShapeType::Box, e.get_shape_type() );
	}

	physics.clear();
	EXPECT_EQ( 0u, physics.get_owned_shape_count() );
}

TEST( PhysicsManagerTest, 本編の作り方の剛体はずれを掛けた位置に置き_摩擦はBulletの既定 )
{
	PhysicsManager physics;

	RigidBodyCreateInfo info;
	info.shape_type = RigidBodyShapeType::Capsule;
	info.shape_size = Vector( 0.25f, 1.25f, 0.f );
	info.mass = 1.f;
	info.transform = at( 1.f, 2.f, 3.f );
	info.offset = at( 0.f, 0.875f, 0.f );
	info.friction = RigidBodyCreateInfo::BulletDefaultFriction;
	info.share_shape = true;
	info.use_default_collision_filter = true;

	auto body = physics.create_rigid_body( info );

	const Vector p = body.get_world_transform( Vector::Zero ).get_position();
	EXPECT_FLOAT_EQ( 1.f, p.x() );
	EXPECT_FLOAT_EQ( 2.875f, p.y() );
	EXPECT_FLOAT_EQ( 3.f, p.z() );

	// 足元に戻す（以前の PhysicsSyncSystem::read_body と同じ）
	EXPECT_FLOAT_EQ( 2.f, body.get_world_transform( Vector( 0.f, -0.875f, 0.f ) ).get_position().y() );

	EXPECT_FLOAT_EQ( 0.5f, body.get_friction() );
	EXPECT_FLOAT_EQ( 1.f, body.get_inv_mass() );
	EXPECT_EQ( RigidBodyShapeType::Capsule, body.get_shape_type() );

	// ECS のテストシーンの既定は摩擦 0.1（以前と同じ）
	EXPECT_FLOAT_EQ( 0.1f, physics.create_rigid_body( RigidBodyCreateInfo() ).get_friction() );
}

TEST( PhysicsManagerTest, 地面は上の面が高さ0に置かれ_clearで消える )
{
	PhysicsManager physics;
	physics.add_ground( Vector( 1000.f, 1.f, 1000.f ) );

	EXPECT_EQ( 1, physics.get_rigid_body_count() );

	const auto hit = physics.ray_test_closest( Vector( 3.f, 10.f, 4.f ), Vector( 3.f, -10.f, 4.f ) );
	ASSERT_TRUE( hit.hit );
	// 大きな箱へのレイは Bullet の計算の誤差が数 cm ある（以前の地面と同じ箱。本編の足場の判定も同じ誤差の中で動いていた）
	EXPECT_NEAR( 0.f, hit.point.y(), 0.05f );
	EXPECT_NEAR( 0.5f, hit.fraction, 0.005f );
	EXPECT_FALSE( hit.entity.has_value() );		// 地面はタグ無し
	EXPECT_NEAR( 1.f, hit.normal.y(), 1e-5f );

	physics.clear();

	EXPECT_EQ( 0, physics.get_rigid_body_count() );
	EXPECT_FALSE( physics.ray_test_closest( Vector( 3.f, 10.f, 4.f ), Vector( 3.f, -10.f, 4.f ) ).hit );
}

TEST( PhysicsManagerTest, OBJの三角形を当たりの形として読み_zを反転する )
{
	// z = 1..3 の四角形（z を反転すると -1..-3）を高さ 2 に置く
	TemporaryObj obj( "blue_sky_physics_manager_test.obj",
		"# 床\n"
		"v -1 2 1\n"
		"v 1 2 1\n"
		"v 1 2 3\n"
		"v -1 2 3\n"
		"f 1 2 3\n"
		"f 1 3 4\n"
		"f 1 2 3 4\n" );	// 四角形の面は使わない（以前と同じ）

	PhysicsManager physics;
	ASSERT_TRUE( physics.load_obj( obj.path.string().c_str() ) );

	EXPECT_EQ( 1, physics.get_rigid_body_count() );
	EXPECT_EQ( 1u, physics.get_owned_shape_count() );

	const auto hit = physics.ray_test_closest( Vector( 0.f, 10.f, -2.f ), Vector( 0.f, -10.f, -2.f ) );
	ASSERT_TRUE( hit.hit );
	EXPECT_NEAR( 2.f, hit.point.y(), 1e-4f );
	EXPECT_FALSE( hit.entity.has_value() );

	// z を反転しているので、ファイルの z = 2 の所には無い
	EXPECT_FALSE( physics.ray_test_closest( Vector( 0.f, 10.f, 2.f ), Vector( 0.f, -10.f, 2.f ) ).hit );

	physics.clear();
	EXPECT_EQ( 0, physics.get_rigid_body_count() );
	EXPECT_EQ( 0u, physics.get_owned_shape_count() );
}

TEST( PhysicsManagerTest, OBJが開けなければfalse_三角形が無ければ例外 )
{
	PhysicsManager physics;

	EXPECT_FALSE( physics.load_obj( "this-file-does-not-exist.obj" ) );

	TemporaryObj empty( "blue_sky_physics_manager_test_empty.obj", "v 0 0 0\nv 1 0 0\n" );
	EXPECT_THROW( physics.load_obj( empty.path.string().c_str() ), std::exception );

	EXPECT_EQ( 0, physics.get_rigid_body_count() );
}

TEST( PhysicsManagerTest, レイは自分を通り抜ける )
{
	PhysicsManager physics;
	physics.add_ground( Vector( 100.f, 1.f, 100.f ) );

	auto me = make_box( physics, at( 0.f, 5.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	me.set_entity_tag( 7 );

	const auto plain = physics.ray_test_closest( Vector( 0.f, 10.f, 0.f ), Vector( 0.f, -10.f, 0.f ) );
	ASSERT_TRUE( plain.hit );
	EXPECT_EQ( Tag( 7 ), plain.entity );
	EXPECT_NEAR( 5.5f, plain.point.y(), 1e-4f );

	const auto excluded = physics.ray_test_closest( Vector( 0.f, 10.f, 0.f ), Vector( 0.f, -10.f, 0.f ), & me );
	ASSERT_TRUE( excluded.hit );
	EXPECT_FALSE( excluded.entity.has_value() );	// 地面
	EXPECT_NEAR( 0.f, excluded.point.y(), 1e-4f );
}

TEST( PhysicsManagerTest, レイは自分と相手の2つを通り抜け_間に何かあれば当たる )
{
	PhysicsManager physics;

	auto me = make_box( physics, at( 0.f, 0.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	auto him = make_box( physics, at( 10.f, 0.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	me.set_entity_tag( 1 );
	him.set_entity_tag( 2 );

	const Vector from( 0.f, 0.f, 0.f ), to( 10.f, 0.f, 0.f );

	EXPECT_TRUE( physics.ray_test_closest( from, to, & me ).hit );					// 相手に当たる
	EXPECT_FALSE( physics.ray_test_closest( from, to, & me, & him ).hit );			// 2 つとも通り抜ける（以前の ClosestNotMeAndHim）

	auto wall = make_box( physics, at( 5.f, 0.f, 0.f ), Vector( 0.1f, 2.f, 2.f ) );
	wall.set_entity_tag( 3 );

	const auto blocked = physics.ray_test_closest( from, to, & me, & him );
	ASSERT_TRUE( blocked.hit );
	EXPECT_EQ( Tag( 3 ), blocked.entity );
	EXPECT_NEAR( 0.49f, blocked.fraction, 1e-4f );
}

TEST( PhysicsManagerTest, レイのfilterはタグで当たる物を選び_通り抜けた先に当たる )
{
	PhysicsManager physics;
	physics.add_ground( Vector( 100.f, 1.f, 100.f ) );

	auto ghost = make_box( physics, at( 0.f, 6.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	auto hard = make_box( physics, at( 0.f, 3.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	ghost.set_entity_tag( 10 );
	hard.set_entity_tag( 11 );

	std::vector< Tag > asked;
	auto filter = [ & ] ( Tag tag ) { asked.push_back( tag ); return tag != Tag( 10 ); };

	const auto hit = physics.ray_test_closest( Vector( 0.f, 10.f, 0.f ), Vector( 0.f, -10.f, 0.f ), nullptr, nullptr, filter );
	ASSERT_TRUE( hit.hit );
	EXPECT_EQ( Tag( 11 ), hit.entity );
	EXPECT_NE( asked.end(), std::find( asked.begin(), asked.end(), Tag( 10 ) ) );	// 通り抜けた物にも訊いた

	// 自分として除いた物には訊かない
	asked.clear();
	physics.ray_test_closest( Vector( 0.f, 10.f, 0.f ), Vector( 0.f, -10.f, 0.f ), & ghost, nullptr, filter );
	EXPECT_EQ( asked.end(), std::find( asked.begin(), asked.end(), Tag( 10 ) ) );

	// 全部通り抜けると地面（タグ無し）に当たる
	const auto ground = physics.ray_test_closest( Vector( 0.f, 10.f, 0.f ), Vector( 0.f, -10.f, 0.f ), nullptr, nullptr, [] ( Tag tag ) { return ! tag; } );
	ASSERT_TRUE( ground.hit );
	EXPECT_FALSE( ground.entity.has_value() );
}

TEST( PhysicsManagerTest, 二つの剛体が接しているかを調べる )
{
	PhysicsManager physics;

	auto a = make_box( physics, at( 0.f, 0.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	auto b = make_box( physics, at( 0.9f, 0.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	auto c = make_box( physics, at( 5.f, 0.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );

	EXPECT_TRUE( physics.contact_pair_test( & a, & b ) );
	EXPECT_FALSE( physics.contact_pair_test( & a, & c ) );
	EXPECT_FALSE( physics.contact_pair_test( & a, nullptr ) );

	RigidBody empty;
	EXPECT_FALSE( physics.contact_pair_test( & a, & empty ) );
}

TEST( PhysicsManagerTest, 剛体と接している組をタグで受け取る )
{
	PhysicsManager physics;
	physics.add_ground( Vector( 100.f, 1.f, 100.f ) );

	auto me = make_box( physics, at( 0.f, 0.4f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );	// 地面にめり込む
	auto other = make_box( physics, at( 0.9f, 0.4f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	auto distant = make_box( physics, at( 50.f, 10.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	me.set_entity_tag( 1 );
	other.set_entity_tag( 2 );
	distant.set_entity_tag( 3 );

	std::vector< std::pair< Tag, Tag > > contacts;
	physics.contact_test( me, [ & ] ( Tag a, Tag b ) { contacts.emplace_back( a, b ); } );

	bool ground = false, pair = false;

	for ( const auto& [ a, b ] : contacts )
	{
		EXPECT_EQ( Tag( 1 ), a );	// 調べた剛体が先
		ground = ground || ! b;
		pair = pair || b == Tag( 2 );
		EXPECT_NE( Tag( 3 ), b );
	}

	EXPECT_TRUE( ground );
	EXPECT_TRUE( pair );
}

TEST( PhysicsManagerTest, 世界の中でめり込んでいる接点ごとに組を受け取る )
{
	PhysicsManager physics;
	physics.add_ground( Vector( 100.f, 1.f, 100.f ) );

	auto box = make_box( physics, at( 0.f, 0.4f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ), 1.f );	// 地面にめり込む動く箱
	box.set_entity_tag( 5 );

	// 接点は物理を進めると作られる
	int count = 0;
	physics.for_each_penetrating_contact( [ & ] ( Tag, Tag ) { count++; } );
	EXPECT_EQ( 0, count );

	physics.step_simulation( 1.f / 60.f );

	std::vector< std::pair< Tag, Tag > > contacts;
	physics.for_each_penetrating_contact( [ & ] ( Tag a, Tag b ) { contacts.emplace_back( a, b ); } );

	ASSERT_FALSE( contacts.empty() );

	for ( const auto& [ a, b ] : contacts )
	{
		EXPECT_TRUE( ( a == Tag( 5 ) && ! b ) || ( ! a && b == Tag( 5 ) ) );
	}
}

TEST( PhysicsManagerTest, 置いた箱が自分以外の数える物に触れるかを調べる )
{
	PhysicsManager physics;

	auto me = make_box( physics, at( 0.f, 0.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	auto wall = make_box( physics, at( 0.f, 0.f, 2.f ), Vector( 1.f, 1.f, 0.1f ) );
	me.set_entity_tag( 1 );
	wall.set_entity_tag( 2 );

	const Vector half( 0.05f, 0.05f, 0.05f );

	// 物体の位置から前（+z）に置いた箱
	EXPECT_TRUE( physics.box_contact_test( at( 0.f, 0.f, 0.f ), Vector( 0.f, 0.f, 1.9f ), half, & me ) );
	EXPECT_FALSE( physics.box_contact_test( at( 0.f, 0.f, 0.f ), Vector( 0.f, 0.f, 1.f ), half, & me ) );

	// 自分に重ねても、自分は数えない
	EXPECT_FALSE( physics.box_contact_test( at( 0.f, 0.f, 0.f ), Vector::Zero, half, & me ) );
	EXPECT_TRUE( physics.box_contact_test( at( 0.f, 0.f, 0.f ), Vector::Zero, half, nullptr ) );

	// 数えない物（filter が false）は触れても数えない。置いた箱そのものはタグ無しとして訊かれる
	std::vector< Tag > asked;
	EXPECT_FALSE( physics.box_contact_test( at( 0.f, 0.f, 0.f ), Vector( 0.f, 0.f, 1.9f ), half, & me, [ & ] ( Tag tag ) { asked.push_back( tag ); return tag != Tag( 2 ); } ) );
	EXPECT_NE( asked.end(), std::find( asked.begin(), asked.end(), Tag() ) );

	// 物体の回転も掛ける（y 軸で 180 度回すと、前は -z）
	Transform turned = at( 0.f, 0.f, 4.f );
	turned.set_rotation( core::math::Quaternion::from_yaw_pitch_roll( 3.14159265f, 0.f, 0.f ) );
	EXPECT_TRUE( physics.box_contact_test( turned, Vector( 0.f, 0.f, 1.9f ), half, & me ) );

	// 余白を含めた半分の大きさ（Bullet の箱の余白 0.04 を含めても、0.05 の箱は 0.05）
	EXPECT_NEAR( 0.05f, PhysicsManager::get_box_half_extents_with_margin( half ).y(), 1e-6f );
}

TEST( PhysicsManagerTest, clearは物体の剛体を消さずに世界から外し_重力を既定に戻す )
{
	PhysicsManager physics;
	physics.set_gravity( Vector( 0.f, -3.711f, 0.f ) );

	auto body = make_box( physics, at( 0.f, 0.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ), 1.f );
	physics.add_ground( Vector( 10.f, 1.f, 10.f ) );
	ASSERT_EQ( 2, physics.get_rigid_body_count() );

	physics.clear();

	EXPECT_EQ( 0, physics.get_rigid_body_count() );
	EXPECT_TRUE( body.is_valid() );		// 持ち主（物体の部品）が後で消す
	EXPECT_FLOAT_EQ( -9.8f, physics.get_gravity().y() );

	body.reset();	// 外れた剛体を消しても壊れない
	EXPECT_EQ( 0, physics.get_rigid_body_count() );
}

TEST( RigidBodyTest, 剛体の性質をメンバ関数で変える )
{
	PhysicsManager physics;
	auto body = make_box( physics, at( 0.f, 0.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ), 1.f );

	EXPECT_TRUE( body.has_contact_response() );
	body.set_no_contact_response( true );
	EXPECT_FALSE( body.has_contact_response() );
	body.set_no_contact_response( false );
	EXPECT_TRUE( body.has_contact_response() );

	EXPECT_FALSE( body.is_kinematic() );
	body.set_kinematic( true );
	EXPECT_TRUE( body.is_kinematic() );
	body.set_kinematic( false );
	EXPECT_FALSE( body.is_kinematic() );

	body.set_mass( 0.f );
	EXPECT_FLOAT_EQ( 0.f, body.get_inv_mass() );
	body.set_mass( 4.f );
	EXPECT_FLOAT_EQ( 0.25f, body.get_inv_mass() );

	body.set_gravity( Vector( 0.f, -1.f, 0.f ) );
	EXPECT_FLOAT_EQ( -1.f, body.get_gravity().y() );

	body.set_friction( 10.f );
	EXPECT_FLOAT_EQ( 10.f, body.get_friction() );

	body.set_angular_factor( 0.f );
	EXPECT_FLOAT_EQ( 0.f, body.get_angular_factor().y() );

	body.set_ccd( 0.2f, 0.001f );
	EXPECT_FLOAT_EQ( 0.2f, body.get_ccd_swept_sphere_radius() );
	EXPECT_FLOAT_EQ( 0.001f, body.get_ccd_motion_threshold() );

	body.set_linear_velocity( Vector( 1.f, 2.f, 3.f ) );
	EXPECT_FLOAT_EQ( 2.f, body.get_linear_velocity().y() );

	EXPECT_FALSE( body.get_entity_tag().has_value() );
	body.set_entity_tag( 0 );
	EXPECT_EQ( Tag( 0 ), body.get_entity_tag() );		// 0 番の Entity もタグ無しと区別する
	body.clear_entity_tag();
	EXPECT_FALSE( body.get_entity_tag().has_value() );
}

TEST( RigidBodyTest, 位置はずれを掛けて送り_ずれを戻して読む )
{
	PhysicsManager physics;
	auto body = make_box( physics, at( 0.f, 0.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ), 1.f );

	body.set_world_transform( at( 1.f, 2.f, 3.f ), Vector( 0.f, 0.5f, 0.f ) );
	EXPECT_FLOAT_EQ( 2.5f, body.get_center_of_mass_position().y() );
	EXPECT_FLOAT_EQ( 2.f, body.get_world_transform( Vector( 0.f, -0.5f, 0.f ) ).get_position().y() );

	body.set_world_transform( at( 4.f, 5.f, 6.f ) );
	EXPECT_FLOAT_EQ( 5.f, body.get_center_of_mass_position().y() );
	EXPECT_FLOAT_EQ( 4.f, body.get_world_transform( Vector::Zero ).get_position().x() );
}

// ---- 位置で動かす剛体（キネマティック。メダル・風船） ----

namespace
{

/// 本編の物体と同じ作り方で、作るときからキネマティックな箱を作る
RigidBody make_kinematic_box( PhysicsManager& physics, const Transform& transform, const Vector& half )
{
	RigidBodyCreateInfo info;
	info.shape_type = RigidBodyShapeType::Box;
	info.shape_size = half;
	info.mass = 1.f;
	info.transform = transform;
	info.offset = Transform::identity();
	info.friction = RigidBodyCreateInfo::BulletDefaultFriction;
	info.share_shape = true;
	info.use_default_collision_filter = true;
	info.is_kinematic = true;

	return physics.create_rigid_body( info );
}

} // namespace

TEST( RigidBodyTest, キネマティックな剛体は重力で落ちず_書いた位置は物理を進めても戻らない )
{
	PhysicsManager physics;
	auto body = make_kinematic_box( physics, at( 0.f, 10.f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );

	EXPECT_TRUE( body.is_kinematic() );

	physics.step_simulation( 1.f / 60.f );
	EXPECT_FLOAT_EQ( 10.f, body.get_center_of_mass_position().y() );

	// 書いた位置は、Bullet が step のたびに動きの状態から読み直しても、そのまま（動きの状態にも書くので）
	body.set_world_transform( at( 3.f, 4.f, 5.f ), Vector( 0.f, 0.5f, 0.f ) );

	for ( int n = 0; n < 5; n++ )
	{
		physics.step_simulation( 1.f / 60.f );
	}

	EXPECT_FLOAT_EQ( 3.f, body.get_center_of_mass_position().x() );
	EXPECT_FLOAT_EQ( 4.5f, body.get_center_of_mass_position().y() );
	EXPECT_FLOAT_EQ( 5.f, body.get_center_of_mass_position().z() );

	// 位置を読むのも同じ（PhysicsManager::get_rigid_body_transform は動きの状態から読む）
	Vector position;
	core::math::Quaternion rotation;
	physics.get_rigid_body_transform( body, position, rotation );
	EXPECT_FLOAT_EQ( 4.5f, position.y() );
}

TEST( PhysicsManagerTest, キネマティックな剛体は動く剛体と当たり_動かない物_ほかのキネマティックな物とは組を作らない )
{
	PhysicsManager physics;
	physics.add_ground( Vector( 100.f, 1.f, 100.f ) );

	// 地面にめり込むキネマティックな箱 2 つ（互いにも重なる）と、それに重なる動く箱
	auto a = make_kinematic_box( physics, at( 0.f, 0.4f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	auto b = make_kinematic_box( physics, at( 0.5f, 0.4f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ) );
	auto dynamic = make_box( physics, at( 0.f, 1.2f, 0.f ), Vector( 0.5f, 0.5f, 0.5f ), 1.f );
	a.set_entity_tag( 1 );
	b.set_entity_tag( 2 );
	dynamic.set_entity_tag( 3 );

	// 当たりの応答を消しても（メダル・風船）、組は作る
	a.set_no_contact_response( true );

	physics.step_simulation( 1.f / 60.f );

	std::vector< std::pair< Tag, Tag > > contacts;
	physics.for_each_penetrating_contact( [ & ] ( Tag x, Tag y ) { contacts.emplace_back( x, y ); } );

	auto has = [ & ] ( Tag x, Tag y )
	{
		return std::any_of( contacts.begin(), contacts.end(), [ & ] ( const auto& c ) { return ( c.first == x && c.second == y ) || ( c.first == y && c.second == x ); } );
	};

	EXPECT_TRUE( has( Tag( 1 ), Tag( 3 ) ) );		// キネマティック × 動く物
	EXPECT_TRUE( has( Tag( 2 ), Tag( 3 ) ) );
	EXPECT_FALSE( has( Tag( 1 ), Tag( 2 ) ) );		// キネマティックどうし
	EXPECT_FALSE( has( Tag( 1 ), Tag() ) );			// キネマティック × 地面（動かない物）
	EXPECT_FALSE( has( Tag( 2 ), Tag() ) );

	// 動く剛体から調べても見つかる（プレイヤーの当たりの調べ方 contact_test）
	std::vector< Tag > touched;
	physics.contact_test( dynamic, [ & ] ( Tag, Tag y ) { touched.push_back( y ); } );
	EXPECT_NE( std::find( touched.begin(), touched.end(), Tag( 1 ) ), touched.end() );

	// 数える（開発用）
	const auto stats = physics.get_stats();
	EXPECT_EQ( 4, stats.collision_objects );
	EXPECT_EQ( 2, stats.kinematic_objects );
}
