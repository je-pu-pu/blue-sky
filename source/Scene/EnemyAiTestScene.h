#pragma once

#include "Scene.h"
#include <core/math/Vector.h>
#include <core/nav/NavMesh.h>
#include <memory>
#include <vector>

namespace core::ecs
{
	class Entity;
	class TransformComponent;
	class CameraComponent;
}

namespace blue_sky::ecs
{
	struct FpsPlayerComponent;
}

namespace blue_sky
{

/**
 * 敵AI（知覚・警戒・パトロール）の縦スライス検証シーン。
 *
 * 地面＋遮蔽ビル＋ウェイポイントを巡回する敵を配置し、FPS プレイヤー（＝侵入者）が
 * 視界錐に入ると 無警戒→不審→捜索→警報 と警戒が上がる様子を可視化する。
 * 視界錐・パトロール経路・最終視認地点を Bullet デバッグ線で描画する。
 */
class EnemyAiTestScene : public Scene
{
public:
	static inline const char_t* name = "enemy_ai_test";

	/// ステージ形状。機構（AI・戦闘・動詞）は共通で、地形/敵/目標の配置だけを切り替える。
	enum class StageKind
	{
		Compact,  ///< 従来の検証用の小さな街（enemy_ai_test）
		Large,    ///< 広くて高低差の大きい街（enemy_ai_test_large）。縦の動詞をテストする
	};

	explicit EnemyAiTestScene( GameMain& game_main, StageKind kind = StageKind::Compact );
	~EnemyAiTestScene() override = default;

	void update() override;
	void render() override;

	bool is_clip_cursor_required() override { return true; }

private:
	/// 敵の種別（doc/動詞とゲームシステム.md 4章）。滞空タイプ（飛行ドローン）はまだ無い。
	enum class EnemyKind
	{
		Ground,   ///< 地上の歩行ロボ。上をほとんど見ない ＝「高いほど安全」を保証する側
		AntiAir,  ///< 対空。地上に居たまま上方を広く見る ＝ 空の「下」からの脅威
	};

	void spawn_enemy( const core::math::Vector& start, const std::vector< core::math::Vector >& waypoints, EnemyKind kind = EnemyKind::Ground );
	void draw_debug();

	void build_stage_compact();  ///< 従来の小さな街を組む（地形・敵・目標）
	void build_stage_large();    ///< 広くて高低差の大きい街を組む（縦の動詞テスト用）

	float nav_ground_half_ = 25.f; ///< navmesh の地面矩形の半径（ステージの広さに合わせる）

	core::ecs::Entity* player_ = nullptr;
	core::ecs::TransformComponent* player_transform_ = nullptr;
	core::ecs::CameraComponent* player_camera_ = nullptr;
	blue_sky::ecs::FpsPlayerComponent* fps_player_ = nullptr;

	std::vector< core::ecs::Entity* > enemies_;
	std::vector< EnemyKind >          enemy_kinds_;   ///< 各敵の種別（enemies_ と同順）

	core::math::Vector player_spawn_;   ///< プレイヤーの初期位置（捕捉時に戻す先）
	float caught_flash_ = 0.f;          ///< 捕捉演出（"CAUGHT" 表示）の残り時間 (s)

	core::ecs::Entity* firework_ = nullptr; ///< 花火（陽動の音源）。使い回す（生成/破棄しない）
	float firework_timer_ = 0.f;            ///< 花火が発音している残り時間 (s)

	core::ecs::Entity* smoke_ = nullptr;    ///< 煙玉（視線を遮る球）。使い回す
	float smoke_timer_ = 0.f;               ///< 煙が有効な残り時間 (s)

	/// 障害物ビル（接地した直方体）。center はワールド中心、half は半径。
	struct Obstacle { core::math::Vector center; core::math::Vector half; };

	std::unique_ptr< core::nav::NavMesh > navmesh_;              ///< パトロール/追跡用 navmesh
	std::vector< Obstacle >           obstacles_;               ///< 建物（障害物）。描画・当たり・navmesh の穴に使う
	std::vector< core::math::Vector > nav_edges_;               ///< navmesh のポリゴン辺（描画用キャッシュ）
	std::vector< core::nav::OffMeshLink > nav_links_;           ///< オフメッシュ接続（地上↔屋上。描画用）

	/// 直方体を1つ生成する（描画スケール＋静的コリジョン）。
	/// texture_path を渡すと lit シェーダ＋そのテクスチャ（ビル外壁など）、null なら単色。
	core::ecs::Entity* spawn_solid_box( const core::math::Vector& center, const core::math::Vector& half, const core::Color& color, const char_t* texture_path = nullptr );

	/// 階段を作る（low→high の段状ボックス＝見た目/当たり ＋ オフメッシュ接続）。
	/// 水平移動は x/z の大きい方の軸に沿う（軸並行）。敵はリンクを 3D で渡って昇降する。
	void add_stairs( const core::math::Vector& low, const core::math::Vector& high, float width, const core::Color& color );

	/// 橋を作る（a→b の水平スラブ＝見た目/当たり ＋ オフメッシュ接続）。a.y と b.y は同じ高さ。
	void add_bridge( const core::math::Vector& a, const core::math::Vector& b, float width, const core::Color& color );

	void build_navmesh();               ///< 地面＋建物＋コネクタ(nav_links_)から navmesh を焼く

	core::ecs::Entity* carried_ = nullptr;  ///< 運搬中の死体（ダウン敵）。null なら運搬なし

	void check_caught();                ///< 警戒中の敵に接近されたらスポーンへ戻す
	void teleport_player( const core::math::Vector& feet ); ///< プレイヤーを剛体ごと足元 feet へ瞬間移動する（速度は消す）

	// --- 発見後の脅威・戦闘（プレイヤー HP・敵の射撃） ---
	float player_hp_          = 100.f;  ///< プレイヤーの体力
	float player_hp_max_      = 100.f;
	float combat_fire_interval_ = 0.7f; ///< 敵の発砲間隔 (s)
	float combat_shot_damage_ = 14.f;   ///< 1発の被ダメージ
	float combat_regen_       = 8.f;    ///< 見られていない時の回復 (/s)
	float hit_flash_          = 0.f;    ///< 被弾演出の残り (s)
	std::vector< float >              enemy_fire_cd_;  ///< 各敵の発砲クールダウン（enemies_ と同順）
	std::vector< core::math::Vector > combat_tracers_; ///< 狙ってくる敵の目の位置（レーザー線描画用）
	bool  any_alarmed_prev_ = false;    ///< 前フレームに Alarmed の敵がいたか（発見スティングの立ち上がり検出）
	void  respawn_player();             ///< スポーンへ戻す＋HP/警戒/スコアをリセット
	void  update_combat();              ///< 視認中の Alarmed 敵が一定間隔で射撃、HP 0 でやられ

	// --- 縦軸のコスト（落下ダメージ）と「降りる」動詞（傘） ---
	// 高さに代償が無いと「飛ぶ＝有限資源で一時的な自由を買う」（doc/動詞とゲームシステム.md 2章）が
	// 成立しない。落下ダメージがジェット燃料に意味を与え、傘がその出口になる。
	float fall_safe_speed_   = 8.f;     ///< この着地速度までは無傷 (m/s ≒ 3.3m 落下)
	float fall_lethal_speed_ = 17.f;    ///< この着地速度で満タンから即死 (m/s ≒ 15m 落下)
	float fall_flash_        = 0.f;     ///< 落下ダメージ演出の残り (s)
	int   umbrella_count_    = 3;       ///< 傘の所持数（空中で開くたびに 1 消費）
	int   umbrella_start_    = 3;       ///< 開始時／やり直し時の所持数
	int   umbrella_max_      = 5;       ///< 所持上限（拾い物で補給）
	void  update_umbrella();            ///< 傘の開閉入力と所持数を処理
	void  update_fall_damage();         ///< 着地衝撃 (landing_impact) を HP ダメージへ変換

	/// 事前ロード済みの 3D 音を、ワールド座標を与えてワンショット再生（方向＋距離）。
	void play_sound_3d( const char* sound_name, const core::math::Vector& pos );

	// --- 3D サウンド確認用ビーコン: 自分の位置にループ音を落とし、離れて回ると左右/距離が分かる ---
	bool               beacon_on_ = false;
	core::math::Vector beacon_pos_;

	void update_firework();             ///< 花火の投擲入力と発音タイマを処理
	void update_smoke();                ///< 煙玉の投擲入力と有効タイマを処理
	void try_neutralize();              ///< 背後・近接の敵を無力化（ダウン）する

	// --- 目標収集ゲーム（要所の黄色い立方体を拾う→+1→次へ。連続30でクリア。捕まると 0） ---
	core::ecs::Entity* target_ = nullptr;               ///< 収集目標（黄色い小立方体）
	std::vector< core::math::Vector > target_points_;   ///< 目標の出現候補（要所の中心位置）
	std::size_t current_target_ = 0;                    ///< 現在の目標位置の添字
	int   score_ = 0;                                   ///< 連続獲得ポイント（捕まると 0）
	bool  cleared_ = false;                             ///< 連続30で true
	float target_spin_ = 0.f;                           ///< 目標の回転角（視認性）
	void  place_target();                               ///< 目標を現在の要所へ配置
	void  update_target();                              ///< 目標の回転・収集判定・次への移動

}; // class EnemyAiTestScene

} // namespace blue_sky
