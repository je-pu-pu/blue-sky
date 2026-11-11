#include "EnemyAiTestScene.h"

#include <blue_sky/SceneManager.h>
#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/Input.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RenderComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>

#include <blue_sky/ecs/component/FpsPlayerComponent.h>
#include <blue_sky/ecs/component/PerceivableComponent.h>
#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <blue_sky/ecs/component/AlertComponent.h>
#include <blue_sky/ecs/component/PatrolComponent.h>
#include <blue_sky/ecs/component/SmokeComponent.h>

#include <core/ecs/system/RenderSystem.h>
#include <core/ecs/system/PhysicsSystem.h>
#include <blue_sky/ecs/system/FpsPlayerControlSystem.h>
#include <blue_sky/ecs/system/TargetRegistrySystem.h>
#include <blue_sky/ecs/system/PerceptionSystem.h>
#include <blue_sky/ecs/system/AlertSystem.h>
#include <blue_sky/ecs/system/PatrolSystem.h>
#include <blue_sky/ecs/system/AlarmPropagationSystem.h>
#include <blue_sky/ecs/system/SmokeRegistrySystem.h>

#include <blue_sky/ai/EnemyPerception.h>

#include <core/TimeManager.h>
#include <core/input/InputManager.h>
#include <core/physics/PhysicsManager.h>
#include <core/math/Quaternion.h>
#include <core/graphics/Direct3D11/BulletDebugDraw.h>

#include <game/MainLoop.h>

#include <imgui.h>

#include <sstream>
#include <cmath>

namespace blue_sky
{

using RigidBodyShapeType = core::physics::RigidBodyShapeType;
using core::ecs::TransformComponent;
using core::ecs::CameraComponent;
using core::ecs::ModelComponent;
using core::ecs::RenderComponent;
using core::ecs::RigidBodyComponent;

namespace
{

/// 警戒状態ごとの色（白/灰＝無警戒, 黄＝不審, 橙＝捜索, 赤＝警報）
core::Vector state_color( ai::AlertState s )
{
	switch ( s )
	{
	case ai::AlertState::Suspicious: return core::Vector( 1.f, 1.f, 0.f );
	case ai::AlertState::Searching:  return core::Vector( 1.f, 0.5f, 0.f );
	case ai::AlertState::Alarmed:    return core::Vector( 1.f, 0.f, 0.f );
	case ai::AlertState::Unaware:
	default:                         return core::Vector( 0.5f, 0.5f, 0.5f );
	}
}

const char* state_name( ai::AlertState s )
{
	switch ( s )
	{
	case ai::AlertState::Suspicious: return "SUSPICIOUS";
	case ai::AlertState::Searching:  return "SEARCHING";
	case ai::AlertState::Alarmed:    return "ALARMED";
	case ai::AlertState::Unaware:
	default:                         return "UNAWARE";
	}
}

} // namespace

EnemyAiTestScene::EnemyAiTestScene( GameMain& game_main )
	: Scene( game_main )
{
	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	// Bullet デバッグ描画（LOS レイの遮蔽やコーン可視化に使う）
	get_physics_manager()->set_debug_drawer( get_bullet_debug_draw() );

	// System を追加（実行順が重要: 対象収集 → 知覚 → 警戒 → 移動 → 物理 → 描画）
	get_entity_manager()->add_system< core::ecs::PhysicsSystem >( 5, get_physics_manager() );
	get_entity_manager()->add_system< blue_sky::ecs::FpsPlayerControlSystem >( 10, get_input(), get_physics_manager(), get_time_manager() );
	get_entity_manager()->add_system< blue_sky::ecs::TargetRegistrySystem >( 20 );
	get_entity_manager()->add_system< blue_sky::ecs::SmokeRegistrySystem >( 25 );
	get_entity_manager()->add_system< blue_sky::ecs::PerceptionSystem >( 30, get_physics_manager() );
	get_entity_manager()->add_system< blue_sky::ecs::AlertSystem >( 40, get_time_manager() );
	get_entity_manager()->add_system< blue_sky::ecs::AlarmPropagationSystem >( 45 );
	get_entity_manager()->add_system< blue_sky::ecs::PatrolSystem >( 50, get_time_manager() );
	get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000, get_graphics_manager(), get_time_manager() );

	// --- プレイヤー（侵入者） ---
	player_ = get_entity_manager()->create_entity();
	player_transform_ = player_->add_component< TransformComponent >();
	player_transform_->transform.set_identity();
	player_transform_->transform.set_position( Vector( 0.f, 2.f, -10.f ) );
	player_spawn_ = Vector( 0.f, 2.f, -10.f );

	fps_player_ = player_->add_component< blue_sky::ecs::FpsPlayerComponent >();
	fps_player_->move_speed = 5.f;
	fps_player_->fly_enabled = true;  // ジェット（滞空）を有効化。空中で Space 長押しで上昇/滞空

	auto* player_rb = player_->add_component< RigidBodyComponent >();
	player_rb->shape_type = RigidBodyShapeType::Capsule;
	player_rb->shape_size = Vector( 0.3f, 1.0f, 0.f );
	player_rb->offset = Vector( 0.f, 0.8f, 0.f );
	player_rb->mass = 70.f;
	player_rb->angular_factor = Vector( 0.f, 0.f, 0.f );
	player_rb->friction = 0.1f;
	player_rb->collision_group = core::physics::CollisionGroup::Player;
	player_rb->collision_mask = core::physics::CollisionGroup::All;

	player_camera_ = player_->add_component< CameraComponent >();
	player_camera_->eye_offset = Vector( 0.f, 1.5f, 0.f );

	// プレイヤーを「知覚され得る対象」にする（敵はプレイヤー型を知らず、このタグで見る）
	player_->add_component< blue_sky::ecs::PerceivableComponent >();

	get_graphics_manager()->set_main_camera_info( player_transform_, player_camera_ );

	// --- 地面 ---
	{
		auto* e = get_entity_manager()->create_entity();
		auto* t = e->add_component< TransformComponent >();
		t->transform.set_identity();
		e->add_component< RenderComponent >();
		auto* m = e->add_component< ModelComponent >();
		m->model = get_graphics_manager()->get_model_handle( get_graphics_manager()->load_model( "ground" ) );

		auto* rb = e->add_component< RigidBodyComponent >();
		rb->shape_type = RigidBodyShapeType::Box;
		rb->shape_size = Vector( 500.f, 1.f, 500.f );
		rb->offset = Vector( 0.f, -1.f, 0.f );
		rb->mass = 0.f;
	}

	// --- ステージ: 合成ビル（複数箱の組み合わせ）＋コネクタ（階段/橋） ---
	// center.y = half.y で底面を地面(y=0)に接地。描画・当たり・navmesh の穴が一致する。
	const Color wall_color( 0.62f, 0.62f, 0.66f, 1.f );  // ビル本体（冷たいグレー）
	const Color walk_color( 0.72f, 0.68f, 0.56f, 1.f );  // 通路（階段/橋・温かいグレー）

	// 合成ビルは階層の箱の「間に隙間」を空けて置く → コネクタ（階段/橋）は隙間だけを跨ぎ、
	// 屋上には乗り上げない。屋上は各箱の上面（別々の島）で、通行はオフメッシュ接続が担う。
	obstacles_ = {
		// 合成ビルA（西）: 2F 屋上 y3 x[-13,-9] ／ 隙間 x[-9,-7] ／ 3F 屋上 y5 x[-7,-3]
		{ Vector( -11.f, 1.5f, 12.f ), Vector( 2.0f, 1.5f, 2.0f ) },  // A-2F
		{ Vector(  -5.f, 2.5f, 12.f ), Vector( 2.0f, 2.5f, 2.0f ) },  // A-3F
		// 合成ビルB（東）: 3F 屋上 y5 x[3,7] ／ 隙間 x[7,9] ／ 2F 屋上 y3 x[9,13]
		{ Vector(   5.f, 2.5f, 12.f ), Vector( 2.0f, 2.5f, 2.0f ) },  // B-3F（西・橋側）
		{ Vector(  11.f, 1.5f, 12.f ), Vector( 2.0f, 1.5f, 2.0f ) },  // B-2F（東）
		// 遮蔽ビル（単箱・複数の高さ）
		{ Vector(  -8.f, 3.0f,  4.f ), Vector( 2.0f, 3.0f, 2.0f ) },  // h6
		{ Vector(   8.f, 2.5f,  4.f ), Vector( 2.0f, 2.5f, 2.0f ) },  // h5
		{ Vector(   0.f, 1.5f, 17.f ), Vector( 3.0f, 1.5f, 2.0f ) },  // h3（低い横長）
		{ Vector(   4.f, 1.0f, -2.f ), Vector( 1.5f, 1.0f, 1.5f ) },  // h2（低い遮蔽）
	};
	for ( const auto& o : obstacles_ )
	{
		spawn_solid_box( o.center, o.half, wall_color );
	}

	// コネクタ（見た目の階段/橋 ＝ 縁と縁の隙間だけ）＋ オフメッシュ接続（端点は屋上の内側へ inset）。
	// 複数ルート: 屋上ネットワークへは 西(A-2F) と 東(B-2F) の2入口＝ループ。h6 遮蔽は見張り台。
	add_stairs( Vector( -11.f, 0.f, 18.f ), Vector( -11.f, 3.f, 14.f ), 2.5f, walk_color ); // 地上→A-2F（北）
	add_stairs( Vector(  -9.f, 3.f, 12.f ), Vector(  -7.f, 5.f, 12.f ), 2.0f, walk_color ); // A-2F→A-3F（隙間）
	add_bridge( Vector(  -3.f, 5.f, 12.f ), Vector(   3.f, 5.f, 12.f ), 2.0f, walk_color ); // A-3F→B-3F（橋）
	add_stairs( Vector(   7.f, 5.f, 12.f ), Vector(   9.f, 3.f, 12.f ), 2.0f, walk_color ); // B-3F→B-2F（隙間）
	add_stairs( Vector(  11.f, 0.f, 18.f ), Vector(  11.f, 3.f, 14.f ), 2.5f, walk_color ); // 地上→B-2F（北・2つ目の入口＝ループ）
	add_stairs( Vector(  -8.f, 0.f, -4.f ), Vector(  -8.f, 6.f,  2.f ), 2.5f, walk_color ); // 地上→h6 遮蔽の屋上（見張り台）

	// 屋上の遮蔽物（腰高の壁/箱＝敵の視線を遮るカバー。当たり＋LOS 遮蔽。navmesh には焼かない）。
	// リンク端点（z12）を避けて縁寄りに置く。敵はキネマティックなので素通りするが、プレイヤーの
	// 遮蔽・隠れ場所になり、屋上での見つかりにくさを作る。
	const Color cover_color( 0.48f, 0.50f, 0.52f, 1.f );
	auto cover = [&]( float cx, float roof, float cz, float hx, float hz, float h )
	{
		spawn_solid_box( Vector( cx, roof + h * 0.5f, cz ), Vector( hx, h * 0.5f, hz ), cover_color );
	};
	cover(  -5.f, 5.f, 13.4f, 1.6f, 0.2f, 1.0f ); // A-3F 北縁の壁
	cover( -6.2f, 5.f, 10.6f, 0.5f, 0.5f, 1.2f ); // A-3F 南西の箱
	cover(   5.f, 5.f, 10.6f, 1.6f, 0.2f, 1.0f ); // B-3F 南縁の壁
	cover(  6.2f, 5.f, 13.4f, 0.5f, 0.5f, 1.2f ); // B-3F 北東の箱
	cover( -11.f, 3.f, 11.0f, 1.6f, 0.2f, 1.0f ); // A-2F の壁
	cover(  11.f, 3.f, 13.0f, 1.6f, 0.2f, 1.0f ); // B-2F の壁
	cover(  -9.f, 6.f,  5.0f, 0.5f, 0.5f, 1.0f ); // h6 見張り台の箱

	// --- 敵（ウェイポイント巡回） ---
	spawn_enemy(
		Vector( 0.f, 0.f, 8.f ),
		{ Vector( -1.f, 0.f, 8.f ), Vector( 1.f, 0.f, 8.f ) } );

	spawn_enemy(
		Vector( 0.f, 0.f, 0.f ),
		{ Vector( 0.f, 0.f, 0.f ), Vector( 0.f, 0.f, 8.f ), Vector( 1.f, 0.f, 8.f ), Vector( 1.f, 0.f, 0.f ) } );

	// 地上→A-2F→A-3F を階段で上って屋上へ向かう敵（階層間コネクタの実証）
	spawn_enemy(
		Vector( -11.f, 0.f, 20.f ),
		{ Vector( -11.f, 0.f, 20.f ), Vector( -5.f, 5.f, 12.f ) } );

	// A-3F ↔ B-3F を橋で往復する敵（ビル間コネクタの実証）
	spawn_enemy(
		Vector( -5.f, 5.f, 12.f ),
		{ Vector( -5.f, 5.f, 12.f ), Vector( 5.f, 5.f, 12.f ) } );

	// 花火（陽動の音源）。生成/破棄はせず使い回す。普段は地下へ退避し無音。
	firework_ = get_entity_manager()->create_entity();
	{
		auto* t = firework_->add_component< TransformComponent >();
		t->transform.set_identity();
		t->transform.set_position( Vector( 0.f, -1000.f, 0.f ) );

		auto* pv = firework_->add_component< blue_sky::ecs::PerceivableComponent >();
		pv->emitting_noise = false;
		pv->noise_loudness = 2.5f; // 花火は大音量（広く引きつける）
	}

	// 煙玉（視線を遮る球）。使い回す。普段は地下へ退避。
	smoke_ = get_entity_manager()->create_entity();
	{
		auto* t = smoke_->add_component< TransformComponent >();
		t->transform.set_identity();
		t->transform.set_position( Vector( 0.f, -1000.f, 0.f ) );

		auto* sm = smoke_->add_component< blue_sky::ecs::SmokeComponent >();
		sm->radius = 3.5f;
	}

	// --- 目標収集ゲーム: 要所（地上/各屋上/橋）に黄色い小立方体。取ると +1 で次へ。連続30でクリア ---
	// 位置は「拾う立方体の中心」。屋上/橋は面より少し上、地上は浮かせて視認しやすくする。
	target_points_ = {
		Vector( -14.f, 0.6f,  0.f ),   // 地上（南西）
		Vector(  -5.f, 5.6f, 12.f ),   // A-3F 屋上
		Vector(  14.f, 0.6f, 20.f ),   // 地上（北東）
		Vector(   0.f, 5.6f, 12.f ),   // 橋の上
		Vector(   5.f, 5.6f, 12.f ),   // B-3F 屋上
		Vector( -11.f, 3.6f, 12.f ),   // A-2F 屋上
		Vector(   0.f, 0.6f, -4.f ),   // 地上（南）
		Vector(  11.f, 3.6f, 12.f ),   // B-2F 屋上
		Vector(  -8.f, 6.6f,  4.f ),   // 遮蔽 h6 屋上
		Vector(   0.f, 3.6f, 17.f ),   // 遮蔽 h3 屋上
		Vector( -14.f, 0.6f, 20.f ),   // 地上（北西）
		Vector(   8.f, 5.6f,  4.f ),   // 遮蔽 h5 屋上
	};
	target_ = get_entity_manager()->create_entity();
	{
		auto* t = target_->add_component< TransformComponent >();
		t->transform.set_identity();
		t->scale = Vector( 0.5f, 0.5f, 0.5f );   // 0.5m の小立方体

		target_->add_component< RenderComponent >();
		auto* m = target_->add_component< ModelComponent >();
		auto* model  = get_graphics_manager()->clone_model( get_graphics_manager()->load_model( "box-1x1x1" ) );
		auto* shader = get_graphics_manager()->clone_shader( get_graphics_manager()->get_shader( "lit_unicolor" ) ); // 単色＋面陰影（ビルと同じ）
		shader->set_color( "color", Color( 1.f, 0.85f, 0.1f, 1.f ) ); // 黄色
		model->set_shader_at( 0, shader );
		m->model = get_graphics_manager()->get_model_handle( model );
	}
	place_target();

	build_navmesh();
}

void EnemyAiTestScene::place_target()
{
	if ( ! target_ || target_points_.empty() )
	{
		return;
	}
	const Vector p = target_points_[ current_target_ % target_points_.size() ];
	// box-1x1x1 は底面原点。scale 0.5 で高さ 0.5 なので、中心を p に合わせるには底を 0.25 下げる。
	if ( auto* t = target_->get_component< TransformComponent >() )
	{
		t->transform.set_position( Vector( p.x(), p.y() - 0.25f, p.z() ) );
	}
}

void EnemyAiTestScene::update_target()
{
	if ( ! target_ || target_points_.empty() )
	{
		return;
	}
	const float dt = get_time_manager()->get_delta_time();

	// 視認性のためゆっくり回転（クリア後も回し続けてよいが、位置は地下へ退避済み）
	target_spin_ += dt * 1.5f;
	if ( auto* t = target_->get_component< TransformComponent >() )
	{
		core::math::Quaternion q;
		q.set_yaw_pitch_roll( target_spin_, 0.f, 0.f );
		t->transform.set_rotation( q );
	}

	if ( cleared_ )
	{
		return;
	}

	// 収集判定: プレイヤーが目標の中心へ十分近づいたら +1 して次の要所へ
	const Vector ppos = player_transform_->transform.get_position();
	const Vector tp   = target_points_[ current_target_ % target_points_.size() ];
	if ( ( tp - ppos ).length() <= 1.6f )
	{
		++score_;
		if ( score_ >= 30 )
		{
			cleared_ = true;
			if ( auto* t = target_->get_component< TransformComponent >() )
			{
				t->transform.set_position( Vector( 0.f, -1000.f, 0.f ) ); // クリア: 目標を隠す
			}
			return;
		}
		current_target_ = ( current_target_ + 1 ) % target_points_.size();
		place_target();
	}
}

void EnemyAiTestScene::spawn_enemy( const Vector& start, const std::vector< Vector >& waypoints )
{
	auto* e = get_entity_manager()->create_entity();

	auto* t = e->add_component< TransformComponent >();
	t->transform.set_identity();
	t->transform.set_position( start );

	// 見た目（プレースホルダの箱＝ロボット）
	e->add_component< RenderComponent >();
	auto* m = e->add_component< ModelComponent >();
	m->model = get_graphics_manager()->get_model_handle( get_graphics_manager()->load_model( "box-1x1x1" ) );

	auto* perception = e->add_component< blue_sky::ecs::PerceptionComponent >();
	perception->eye_height = 1.0f;
	perception->cone.view_distance      = 18.f;
	perception->cone.horizontal_half_fov = 1.0472f; // 60deg
	perception->cone.vertical_up_limit  = 0.3491f;  // 20deg

	e->add_component< blue_sky::ecs::AlertComponent >();

	auto* patrol = e->add_component< blue_sky::ecs::PatrolComponent >();
	patrol->waypoints = waypoints;
	patrol->move_speed = 2.5f;

	enemies_.push_back( e );
}

core::ecs::Entity* EnemyAiTestScene::spawn_solid_box( const Vector& center, const Vector& half, const Color& color )
{
	auto* e = get_entity_manager()->create_entity();

	auto* t = e->add_component< TransformComponent >();
	t->transform.set_identity();
	// box-1x1x1 は底面が原点(y∈[0,1])。中心が center に来るよう底を下げて置く。
	t->transform.set_position( Vector( center.x(), center.y() - half.y(), center.z() ) );
	t->scale = half * 2.f; // box-1x1x1（1辺1）を 2*half に拡大 → 直方体

	e->add_component< RenderComponent >();
	auto* m = e->add_component< ModelComponent >();
	auto* model  = get_graphics_manager()->clone_model( get_graphics_manager()->load_model( "box-1x1x1" ) );
	auto* shader = get_graphics_manager()->clone_shader( get_graphics_manager()->get_shader( "lit_unicolor" ) ); // 単色＋面陰影
	shader->set_color( "color", color );
	model->set_shader_at( 0, shader );
	m->model = get_graphics_manager()->get_model_handle( model );

	// 静的コリジョン。transform を底へ下げたぶん offset で中心を center に戻す。
	auto* rb = e->add_component< RigidBodyComponent >();
	rb->shape_type = RigidBodyShapeType::Box;
	rb->shape_size = half;
	rb->offset = Vector( 0.f, half.y(), 0.f );
	rb->mass = 0.f;

	return e;
}

void EnemyAiTestScene::add_stairs( const Vector& low, const Vector& high, float width, const Color& color )
{
	// 見た目/当たり: 低い側→高い側を段状のソリッドボックスで敷く（各段は下端から踏み面まで）。
	// 上り/下りどちらの向きで呼ばれても正しく積むため、y で低い方(lo)・高い方(hi)に並べ替える。
	// （並べ替えないと下り呼び出しで段の高さが負になり、箱が反転＝法線裏返りして表示が壊れる）
	const Vector& lo = ( low.y() <= high.y() ) ? low : high;
	const Vector& hi = ( low.y() <= high.y() ) ? high : low;

	const float dx = hi.x() - lo.x();
	const float dz = hi.z() - lo.z();
	const float H  = hi.y() - lo.y();                 // 常に >= 0
	const bool  along_x = std::abs( dx ) >= std::abs( dz );
	const float L   = along_x ? std::abs( dx ) : std::abs( dz );
	const float sgn = along_x ? ( dx >= 0.f ? 1.f : -1.f ) : ( dz >= 0.f ? 1.f : -1.f );
	const int   N   = std::max( 4, static_cast< int >( std::round( H / 0.25f ) ) );  // 蹴上げ ~0.25m

	for ( int i = 0; i < N; ++i )
	{
		const float tread_h = lo.y() + H * static_cast< float >( i + 1 ) / N;   // その段の踏み面高さ
		const float t       = ( static_cast< float >( i ) + 0.5f ) / N;         // run 上の中心
		float cx = lo.x(), cz = lo.z(), hx, hz;
		if ( along_x ) { cx = lo.x() + sgn * L * t; hx = L / ( 2.f * N ); hz = width * 0.5f; }
		else           { cz = lo.z() + sgn * L * t; hz = L / ( 2.f * N ); hx = width * 0.5f; }
		// 底を下端より少し下へ埋めて、屋上/地面の上面との Z ファイティングを避ける。
		const float bottom = lo.y() - 0.06f;
		const float cy = ( bottom + tread_h ) * 0.5f;
		const float hy = ( tread_h - bottom ) * 0.5f;   // 常に > 0（反転しない）
		spawn_solid_box( Vector( cx, cy, cz ), Vector( hx, hy, hz ), color );
	}

	// 見た目の階段は lo→hi（縁と縁＝gap）だけを占める。オフメッシュ接続の端点は
	// その外側（各歩行面の内側）へ inset して置く＝屋上に乗り上げず連結だけ確保する。
	const float hlen = std::sqrt( dx * dx + dz * dz );
	const Vector dir = ( hlen > 1e-4f ) ? Vector( dx / hlen, 0.f, dz / hlen ) : Vector( 0.f, 0.f, 1.f );
	const float inset = 1.0f;
	const Vector link_lo( lo.x() - dir.x() * inset, lo.y(), lo.z() - dir.z() * inset );
	const Vector link_hi( hi.x() + dir.x() * inset, hi.y(), hi.z() + dir.z() * inset );
	nav_links_.push_back( { link_lo, link_hi, 0.8f, true } );
}

void EnemyAiTestScene::add_bridge( const Vector& a, const Vector& b, float width, const Color& color )
{
	// 見た目/当たり: a→b（屋上の縁と縁＝gap）だけを跨ぐ薄い水平スラブ。屋上には乗せない。
	const float dx = b.x() - a.x();
	const float dz = b.z() - a.z();
	const bool  along_x = std::abs( dx ) >= std::abs( dz );
	const float L  = along_x ? std::abs( dx ) : std::abs( dz );
	const float cx = ( a.x() + b.x() ) * 0.5f;
	const float cz = ( a.z() + b.z() ) * 0.5f;
	const float hy = 0.15f;
	const float hx = along_x ? L * 0.5f : width * 0.5f;
	const float hz = along_x ? width * 0.5f : L * 0.5f;
	// 上面を屋上より少し上（+0.04）にして Z ファイティングを避ける。
	const float top = a.y() + 0.04f;
	spawn_solid_box( Vector( cx, top - hy, cz ), Vector( hx, hy, hz ), color );

	// 手すり（両脇の低い壁）。橋の縁に沿って立て、落下防止＋視線を部分的に遮る（当たり＋LOS 遮蔽）。
	const float rail_h = 0.5f, rail_t = 0.07f;
	if ( along_x )
	{
		spawn_solid_box( Vector( cx, a.y() + rail_h * 0.5f, cz + width * 0.5f ), Vector( hx, rail_h * 0.5f, rail_t ), color );
		spawn_solid_box( Vector( cx, a.y() + rail_h * 0.5f, cz - width * 0.5f ), Vector( hx, rail_h * 0.5f, rail_t ), color );
	}
	else
	{
		spawn_solid_box( Vector( cx + width * 0.5f, a.y() + rail_h * 0.5f, cz ), Vector( rail_t, rail_h * 0.5f, hz ), color );
		spawn_solid_box( Vector( cx - width * 0.5f, a.y() + rail_h * 0.5f, cz ), Vector( rail_t, rail_h * 0.5f, hz ), color );
	}

	// オフメッシュ接続の端点は縁の外側（各屋上の内側）へ inset して置く＝屋上に乗せず連結を確保。
	const float hlen = std::sqrt( dx * dx + dz * dz );
	const Vector dir = ( hlen > 1e-4f ) ? Vector( dx / hlen, 0.f, dz / hlen ) : Vector( 1.f, 0.f, 0.f );
	const float inset = 1.0f;
	const Vector link_a( a.x() - dir.x() * inset, a.y(), a.z() - dir.z() * inset );
	const Vector link_b( b.x() + dir.x() * inset, b.y(), b.z() + dir.z() * inset );
	nav_links_.push_back( { link_a, link_b, 0.9f, true } );
}

void EnemyAiTestScene::build_navmesh()
{
	// 歩行ジオメトリを三角形で組む: 地面の広い矩形 ＋ 建物ボックス（障害物）
	std::vector< float > verts;
	std::vector< int >   tris;

	auto add_vertex = [&]( float x, float y, float z ) -> int
	{
		const int idx = static_cast< int >( verts.size() / 3 );
		verts.push_back( x ); verts.push_back( y ); verts.push_back( z );
		return idx;
	};
	auto add_tri = [&]( int a, int b, int c )
	{
		tris.push_back( a ); tris.push_back( b ); tris.push_back( c );
	};

	// 地面（-25..25, y=0）。上向き法線になる巻き順。
	{
		const int v0 = add_vertex( -25.f, 0.f, -25.f );
		const int v1 = add_vertex(  25.f, 0.f, -25.f );
		const int v2 = add_vertex(  25.f, 0.f,  25.f );
		const int v3 = add_vertex( -25.f, 0.f,  25.f );
		add_tri( v0, v2, v1 );
		add_tri( v0, v3, v2 );
	}

	// 建物（障害物）: 各接地位置に 4×5×4 のボックスを地面から立てる → 足元が navmesh の穴になる
	auto add_box = [&]( float cx, float cz, float hx, float hz, float top )
	{
		const float x0 = cx - hx, x1 = cx + hx, z0 = cz - hz, z1 = cz + hz, y0 = 0.05f, y1 = top;
		const int b0 = add_vertex( x0, y0, z0 ), b1 = add_vertex( x1, y0, z0 ), b2 = add_vertex( x1, y0, z1 ), b3 = add_vertex( x0, y0, z1 );
		const int t0 = add_vertex( x0, y1, z0 ), t1 = add_vertex( x1, y1, z0 ), t2 = add_vertex( x1, y1, z1 ), t3 = add_vertex( x0, y1, z1 );
		add_tri( b0, t0, t1 ); add_tri( b0, t1, b1 );  // 側面 -Z
		add_tri( b1, t1, t2 ); add_tri( b1, t2, b2 );  // 側面 +X
		add_tri( b2, t2, t3 ); add_tri( b2, t3, b3 );  // 側面 +Z
		add_tri( b3, t3, t0 ); add_tri( b3, t0, b0 );  // 側面 -X
		add_tri( t0, t2, t1 ); add_tri( t0, t3, t2 );  // 上面 +Y
	};
	for ( const auto& o : obstacles_ )
	{
		add_box( o.center.x(), o.center.z(), o.half.x(), o.half.z(), o.center.y() + o.half.y() );
	}

	navmesh_ = std::make_unique< core::nav::NavMesh >();

	// コネクタ（階段/橋）のオフメッシュ接続で島（地上・各屋上）を橋渡しする。nav_links_ は
	// add_stairs / add_bridge が ctor で格納済み。傾斜面を navmesh に焼くとエロージョン/二重床で
	// 分断されるため、Detour のオフメッシュ接続（島を繋ぐ標準機能）で確実に連結する。
	// リンク端点はエロージョン後の歩行可能面の内側に置くこと（屋上端に置くと降下時に届かない）。
	navmesh_->set_off_mesh_links( nav_links_ );

	core::nav::NavMeshBuildConfig cfg;
	cfg.agent_radius = 0.5f;
	navmesh_->build( verts.data(), static_cast< int >( verts.size() / 3 ), tris.data(), static_cast< int >( tris.size() / 3 ), cfg );

	// 描画用にポリゴン辺をキャッシュ
	navmesh_->debug_get_poly_edges( nav_edges_ );

	// パトロール System に navmesh を渡す（以降は経路追従になる）
	if ( auto* ps = get_entity_manager()->get_system< blue_sky::ecs::PatrolSystem >() )
	{
		ps->set_navmesh( navmesh_.get() );
	}
}

void EnemyAiTestScene::update()
{
	Scene::update();

	get_graphics_manager()->update();
	get_graphics_manager()->clear_debug_bullet();

	// プレイヤーの発音（R2/ダッシュ＝走行音）。敵は聴覚で拾う。
	if ( auto* pv = player_->get_component< blue_sky::ecs::PerceivableComponent >() )
	{
		pv->emitting_noise = get_input()->press( core::input::Button::R2 );
		pv->noise_loudness = 1.5f;
	}

	update_firework();
	update_smoke();
	try_neutralize();
	update_target();

	get_entity_manager()->update();

	// 警戒中の敵に接近されたら捕捉 → スポーンへ戻す
	check_caught();
	if ( caught_flash_ > 0.f )
	{
		caught_flash_ -= get_time_manager()->get_delta_time();
	}

	// clear の後・render_debug_bullet の前にコーン等を積む
	draw_debug();
}

void EnemyAiTestScene::check_caught()
{
	const Vector ppos = player_transform_->transform.get_position();
	constexpr float catch_radius = 1.8f;

	bool caught = false;
	for ( auto* e : enemies_ )
	{
		auto* alert = e->get_component< blue_sky::ecs::AlertComponent >();
		auto* et    = e->get_component< TransformComponent >();
		if ( ! alert || ! et || alert->state != ai::AlertState::Alarmed )
		{
			continue;
		}

		Vector d = et->transform.get_position() - ppos;
		d.set_y( 0.f );
		if ( d.length() <= catch_radius )
		{
			caught = true;
			break;
		}
	}

	if ( ! caught )
	{
		return;
	}

	// プレイヤーを物理ボディごとスポーンへ戻す（剛体原点は offset ぶん上）
	teleport_player( player_spawn_ );
	carried_ = nullptr; // 捕まったら運搬中の死体は落とす

	// 敵の警戒を解く（全員 無警戒へ）
	for ( auto* e : enemies_ )
	{
		if ( auto* alert = e->get_component< blue_sky::ecs::AlertComponent >() )
		{
			alert->awareness    = 0.f;
			alert->state        = ai::AlertState::Unaware;
			alert->search_timer = 0.f;
		}
	}

	caught_flash_ = 2.0f;

	// 捕まると連続ポイントはリセットし、最初の要所からやり直し（クリア後は据え置き）
	if ( ! cleared_ )
	{
		score_ = 0;
		current_target_ = 0;
		place_target();
	}
}

void EnemyAiTestScene::teleport_player( const Vector& feet )
{
	// 剛体の原点は足元から offset ぶん上。速度・力は消して起こす（瞬間移動・リスポーン用）
	if ( auto* rb = player_->get_component< RigidBodyComponent >(); rb && rb->handle.is_valid() )
	{
		core::math::Transform t;
		t.set_identity();
		t.set_position( feet + rb->offset );

		rb->handle.set_world_transform( t );
		rb->handle.set_motion_state_transform( t );
		rb->handle.set_linear_velocity( Vector( 0.f, 0.f, 0.f ) );
		rb->handle.set_angular_velocity( Vector( 0.f, 0.f, 0.f ) );
		rb->handle.clear_forces();
		rb->handle.activate( true );
	}

	player_transform_->transform.set_position( feet );
}

void EnemyAiTestScene::update_firework()
{
	auto* t  = firework_->get_component< TransformComponent >();
	auto* pv = firework_->get_component< blue_sky::ecs::PerceivableComponent >();
	if ( ! t || ! pv )
	{
		return;
	}

	// 投擲（Q）: プレイヤーの水平前方 8m 地点に花火を出して発音させる
	if ( get_input()->push( core::input::Button::L ) )
	{
		const float  yaw = fps_player_ ? fps_player_->yaw : 0.f;
		const Vector fwd( std::sin( yaw ), 0.f, std::cos( yaw ) );
		Vector land = player_transform_->transform.get_position() + fwd * 8.f;
		land.set_y( 0.2f );

		t->transform.set_position( land );
		pv->emitting_noise = true;
		firework_timer_    = 3.0f;
	}

	// 発音タイマ。尽きたら無音にして地下へ退避（視界にも入らないように）。
	if ( firework_timer_ > 0.f )
	{
		firework_timer_ -= get_time_manager()->get_delta_time();
		if ( firework_timer_ <= 0.f )
		{
			pv->emitting_noise = false;
			t->transform.set_position( Vector( 0.f, -1000.f, 0.f ) );
		}
	}
}

void EnemyAiTestScene::update_smoke()
{
	auto* t = smoke_->get_component< TransformComponent >();
	if ( ! t )
	{
		return;
	}

	// 投擲（E）: プレイヤーの水平前方 8m に煙玉を出す
	if ( get_input()->push( core::input::Button::R ) )
	{
		const float  yaw = fps_player_ ? fps_player_->yaw : 0.f;
		const Vector fwd( std::sin( yaw ), 0.f, std::cos( yaw ) );
		Vector land = player_transform_->transform.get_position() + fwd * 8.f;
		land.set_y( 1.2f ); // 目の高さ付近を覆う
		t->transform.set_position( land );
		smoke_timer_ = 5.0f;
	}

	// 有効タイマ。尽きたら地下へ退避（視線を遮らないように）。
	if ( smoke_timer_ > 0.f )
	{
		smoke_timer_ -= get_time_manager()->get_delta_time();
		if ( smoke_timer_ <= 0.f )
		{
			t->transform.set_position( Vector( 0.f, -1000.f, 0.f ) );
		}
	}
}

void EnemyAiTestScene::try_neutralize()
{
	auto* input = get_input();

	// 運搬中の死体はプレイヤーの前方に追従（毎フレーム・引きずる高さ）
	if ( carried_ )
	{
		if ( auto* ct = carried_->get_component< TransformComponent >() )
		{
			const float  yaw = fps_player_ ? fps_player_->yaw : 0.f;
			const Vector fwd( std::sin( yaw ), 0.f, std::cos( yaw ) );
			Vector p = player_transform_->transform.get_position() + fwd * 1.2f;
			p.set_y( 0.3f );
			ct->transform.set_position( p );
		}
	}

	if ( ! input->push( core::input::Button::L2 ) )
	{
		return; // Z が押された瞬間だけアクション
	}

	// 運搬中なら Z でその場にドロップ
	if ( carried_ )
	{
		carried_ = nullptr;
		return;
	}

	const Vector ppos = player_transform_->transform.get_position();

	// 近くのダウン敵がいれば拾って運搬開始（＝死体隠しの移動手段）
	{
		constexpr float pickup_reach = 2.0f;
		core::ecs::Entity* pick = nullptr;
		float best_pick = pickup_reach;
		for ( auto* e : enemies_ )
		{
			auto* perc = e->get_component< blue_sky::ecs::PerceptionComponent >();
			auto* et   = e->get_component< TransformComponent >();
			if ( ! perc || ! et || ! perc->downed )
			{
				continue;
			}
			Vector d = et->transform.get_position() - ppos;
			d.set_y( 0.f );
			if ( d.length() < best_pick ) { best_pick = d.length(); pick = e; }
		}
		if ( pick )
		{
			carried_ = pick;
			return;
		}
	}

	// 背後・近接の生きた敵を無力化
	constexpr float reach = 2.5f;
	core::ecs::Entity* best = nullptr;
	float best_dist = reach;

	for ( auto* e : enemies_ )
	{
		auto* perc = e->get_component< blue_sky::ecs::PerceptionComponent >();
		auto* et   = e->get_component< TransformComponent >();
		if ( ! perc || ! et || perc->downed )
		{
			continue;
		}

		const Vector epos = et->transform.get_position();
		Vector to_player = ppos - epos;
		to_player.set_y( 0.f );
		const float d = to_player.length();
		if ( d > reach || d < 1e-3f )
		{
			continue;
		}

		// プレイヤーが敵の背後にいるか（敵前方と 敵→プレイヤー が逆向き）
		Vector fwd = et->transform.forward();
		fwd.set_y( 0.f );
		const float flen = fwd.length();
		if ( flen < 1e-3f )
		{
			continue;
		}
		const float along = fwd.dot( to_player ) / ( flen * d ); // -1(真後ろ)..1(正面)
		if ( along > -0.2f )
		{
			continue; // 背後ではない
		}

		if ( d < best_dist )
		{
			best_dist = d;
			best = e;
		}
	}

	if ( ! best )
	{
		return;
	}

	// ダウンさせる: 停止・知覚停止し、他の敵に「発見される対象」になる
	auto* perc  = best->get_component< blue_sky::ecs::PerceptionComponent >();
	auto* alert = best->get_component< blue_sky::ecs::AlertComponent >();
	perc->downed = true;
	if ( alert )
	{
		alert->awareness = 0.f;
		alert->state     = ai::AlertState::Unaware;
	}

	// Perceivable を付与（音は出さない＝視認でのみ発見される）。既にあれば何もしない。
	if ( ! best->get_component< blue_sky::ecs::PerceivableComponent >() )
	{
		auto* pv = best->add_component< blue_sky::ecs::PerceivableComponent >();
		pv->emitting_noise = false;
		pv->noise_loudness = 0.f;
	}
}

void EnemyAiTestScene::draw_debug()
{
	auto* dd = get_bullet_debug_draw();
	if ( ! dd )
	{
		return;
	}

	for ( auto* e : enemies_ )
	{
		auto* transform  = e->get_component< TransformComponent >();
		auto* perception = e->get_component< blue_sky::ecs::PerceptionComponent >();
		auto* alert      = e->get_component< blue_sky::ecs::AlertComponent >();
		auto* patrol     = e->get_component< blue_sky::ecs::PatrolComponent >();
		if ( ! transform || ! perception || ! alert )
		{
			continue;
		}

		// 無力化された敵は錐を描かず、ダウン表示（青い X）のみ
		if ( perception->downed )
		{
			const Vector dp = transform->transform.get_position();
			const core::Vector down_col( 0.2f, 0.5f, 1.f );
			dd->drawLine( dp + Vector( -0.6f, 0.1f, -0.6f ), dp + Vector( 0.6f, 0.1f, 0.6f ), down_col );
			dd->drawLine( dp + Vector( -0.6f, 0.1f,  0.6f ), dp + Vector( 0.6f, 0.1f, -0.6f ), down_col );
			continue;
		}

		const Vector pos     = transform->transform.get_position();
		const Vector forward = transform->transform.forward();
		const core::Vector col  = state_color( alert->state );

		// 視界錐は「床に投影した扇」として描く。
		// 目の高さの水平な扇だと、同じ目線高さの FPS 視点からは真横＝1本の線に見えて
		// 三角形が判別できないため、地面すれすれに描いて見下ろす形で扇が見えるようにする。
		const float floor_y = 0.05f; // 地面のわずか上（z-fighting 回避）
		const Vector apex( pos.x(), floor_y, pos.z() );
		const float  base = std::atan2( forward.x(), forward.z() );
		const float  half = perception->cone.horizontal_half_fov;
		const float  dist = perception->cone.view_distance;
		const int    N    = 12;
		Vector prev_end;
		for ( int i = 0; i <= N; ++i )
		{
			const float a = base - half + ( 2.f * half ) * static_cast< float >( i ) / N;
			const Vector dir( std::sin( a ), 0.f, std::cos( a ) );
			const Vector end = apex + dir * dist;

			dd->drawLine( apex, end, col );         // 稜線（床）
			if ( i > 0 )
			{
				dd->drawLine( prev_end, end, col ); // 弧（床）
			}
			prev_end = end;
		}
		// 敵の体と扇をつなぐ縦線（足元→目の高さ）。錐の原点が敵だと分かるように。
		dd->drawLine( apex, pos + Vector( 0.f, perception->eye_height, 0.f ), col );

		// 正面マーカ（白い矢印＝鼻）。箱のどちらが前かを一目で分かるように前方へ突き出す。
		// 状態色（黄/橙/赤/灰）と区別するため固定の白にする。
		{
			const core::Vector nose_col( 1.f, 1.f, 1.f );
			const Vector chest = pos + Vector( 0.f, 0.6f, 0.f );  // 箱の胸の高さ
			const Vector tip   = chest + forward * 1.2f;          // 前方へ伸ばす
			const Vector side( forward.z(), 0.f, -forward.x() );  // forward の水平右手
			dd->drawLine( chest, tip, nose_col );
			dd->drawLine( tip, tip - forward * 0.35f + side * 0.28f, nose_col );  // 矢じり
			dd->drawLine( tip, tip - forward * 0.35f - side * 0.28f, nose_col );
		}

		// 可聴半径（薄い円・床）。聴覚の届く範囲。
		{
			const core::Vector hear_col( 0.30f, 0.30f, 0.38f );
			const float hr = perception->hearing_radius;
			const int   M  = 24;
			Vector prev_h;
			for ( int i = 0; i <= M; ++i )
			{
				const float a = 6.28318531f * static_cast< float >( i ) / M;
				const Vector p( pos.x() + std::sin( a ) * hr, floor_y, pos.z() + std::cos( a ) * hr );
				if ( i > 0 )
				{
					dd->drawLine( prev_h, p, hear_col );
				}
				prev_h = p;
			}
		}

		// パトロール経路（薄い青の折れ線・床）
		if ( patrol && patrol->waypoints.size() >= 2 )
		{
			const core::Vector path_col( 0.2f, 0.4f, 1.f );
			for ( std::size_t i = 0; i < patrol->waypoints.size(); ++i )
			{
				Vector a = patrol->waypoints[ i ];
				Vector b = patrol->waypoints[ ( i + 1 ) % patrol->waypoints.size() ];
				a.set_y( floor_y );
				b.set_y( floor_y );
				dd->drawLine( a, b, path_col );
			}
		}

		// 現在たどっている navmesh 経路（マゼンタ）
		if ( patrol && patrol->path.size() >= 2 )
		{
			const core::Vector nav_path_col( 1.f, 0.2f, 0.9f );
			for ( std::size_t i = 0; i + 1 < patrol->path.size(); ++i )
			{
				dd->drawLine( patrol->path[ i ], patrol->path[ i + 1 ], nav_path_col );
			}
		}

		// 最終視認地点（警戒中のみ、状態色の縦十字）
		if ( perception->has_last_seen && alert->state != ai::AlertState::Unaware )
		{
			const Vector p = perception->last_seen_position;
			dd->drawLine( p - Vector( 0.5f, 0.f, 0.f ), p + Vector( 0.5f, 0.f, 0.f ), col );
			dd->drawLine( p - Vector( 0.f, 0.f, 0.5f ), p + Vector( 0.f, 0.f, 0.5f ), col );
			dd->drawLine( p, p + Vector( 0.f, 2.f, 0.f ), col );
		}
	}

	// 花火（陽動）の音源マーカー（オレンジ）
	if ( firework_timer_ > 0.f )
	{
		if ( auto* t = firework_->get_component< TransformComponent >() )
		{
			const Vector p = t->transform.get_position();
			const core::Vector fw_col( 1.f, 0.6f, 0.f );
			dd->drawLine( p - Vector( 0.6f, 0.f, 0.f ), p + Vector( 0.6f, 0.f, 0.f ), fw_col );
			dd->drawLine( p - Vector( 0.f, 0.f, 0.6f ), p + Vector( 0.f, 0.f, 0.6f ), fw_col );
			dd->drawLine( p, p + Vector( 0.f, 1.5f, 0.f ), fw_col );
		}
	}

	// 煙玉（視線を遮る球）を薄いグレーの円で描画
	if ( smoke_timer_ > 0.f )
	{
		auto* t  = smoke_->get_component< TransformComponent >();
		auto* sm = smoke_->get_component< blue_sky::ecs::SmokeComponent >();
		if ( t && sm )
		{
			const Vector c = t->transform.get_position();
			const float  r = sm->radius;
			const core::Vector sm_col( 0.70f, 0.70f, 0.75f );
			const int M = 20;
			Vector prev_s;
			for ( int i = 0; i <= M; ++i )
			{
				const float a = 6.28318531f * static_cast< float >( i ) / M;
				const Vector p( c.x() + std::sin( a ) * r, c.y(), c.z() + std::cos( a ) * r );
				if ( i > 0 )
				{
					dd->drawLine( prev_s, p, sm_col );
				}
				prev_s = p;
			}
		}
	}

	// navmesh のポリゴン辺（緑）＝敵が歩ける領域
	{
		const core::Vector nav_col( 0.2f, 0.85f, 0.35f );
		for ( std::size_t i = 0; i + 1 < nav_edges_.size(); i += 2 )
		{
			dd->drawLine( nav_edges_[ i ], nav_edges_[ i + 1 ], nav_col );
		}
	}

	// オフメッシュ接続（水色）＝地上↔屋上の橋渡し。敵はここを 3D で渡って登る。
	{
		const core::Vector link_col( 0.2f, 0.8f, 1.0f );
		for ( const auto& lk : nav_links_ )
		{
			const core::Vector a( lk.start.x(), lk.start.y(), lk.start.z() );
			const core::Vector b( lk.end.x(),   lk.end.y(),   lk.end.z() );
			dd->drawLine( a, b, link_col );
			dd->drawLine( a, a + core::Vector( 0, 0.5f, 0 ), link_col );  // 端点マーカ
			dd->drawLine( b, b + core::Vector( 0, 0.5f, 0 ), link_col );
		}
	}

}

void EnemyAiTestScene::render()
{
	// 描く System（RenderSystem など）を、画面のクリアの後に動かす
	get_entity_manager()->render();

	get_graphics_manager()->render_debug_bullet();

	std::stringstream ss;
	ss << "Enemy AI Test  FPS: " << get_main_loop()->get_last_fps() << "\n";
	if ( cleared_ )
	{
		ss << "*** GAME CLEAR! 30 collected in a row ***\n";
	}
	else
	{
		ss << "SCORE: " << score_ << " / 30   (get the spinning yellow cube; caught = reset)\n";
	}
	if ( caught_flash_ > 0.f )
	{
		ss << "*** CAUGHT! streak reset, sent back to start ***\n";
	}
	ss << "WASD move, C: run, Space: jump / hold in air to fly, Q: firework, E: smoke, Z: neutralize/carry.\n";
	if ( fps_player_ )
	{
		ss << "jet fuel: " << static_cast< int >( fps_player_->fly_fuel * 100.f ) << "%\n";
	}
	if ( carried_ )
	{
		ss << "*** carrying a body (Z to drop) ***\n";
	}

	if ( auto* pv = player_->get_component< blue_sky::ecs::PerceivableComponent >() )
	{
		ss << "player noise: " << ( pv->emitting_noise ? "ON (running)" : "off" ) << "\n";
	}

	for ( std::size_t i = 0; i < enemies_.size(); ++i )
	{
		auto* alert = enemies_[ i ]->get_component< blue_sky::ecs::AlertComponent >();
		auto* perc  = enemies_[ i ]->get_component< blue_sky::ecs::PerceptionComponent >();
		if ( ! alert ) { continue; }
		if ( perc && perc->downed )
		{
			ss << "enemy[" << i << "] DOWNED\n";
			continue;
		}
		ss << "enemy[" << i << "] " << state_name( alert->state )
		   << "  awareness=" << alert->awareness << "\n";
	}

	get_graphics_manager()->draw_text( 10.f, 10.f, 480.f, 200.f, ss.str().c_str(), Color::White );

	// 警報伝播の切替（簡単に ON/OFF できるように）
	ImGui::Begin( "Enemy AI" );
	if ( auto* s = get_entity_manager()->get_system< blue_sky::ecs::AlarmPropagationSystem >() )
	{
		ImGui::Checkbox( "alarm propagation", & s->enabled );
		ImGui::SliderFloat( "propagation radius", & s->radius, 2.f, 30.f );
	}
	ImGui::End();
}

} // namespace blue_sky
