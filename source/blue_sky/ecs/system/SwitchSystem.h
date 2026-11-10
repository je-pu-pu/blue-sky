#pragma once

#include <core/ecs/System.h>
#include <blue_sky/ecs/component/SwitchComponent.h>

#include <optional>

namespace core::sound { class SoundManager; }

namespace blue_sky::ecs
{

/**
 * スイッチ（SwitchComponent）の状態を変え、イベントの受け取り手を呼ぶ
 *
 * 以前の BaseSwitch・Switch・AreaSwitch のメンバ関数（do_switch・do_break・turn_on・turn_off・on_collide_with・add_event_handler）。
 * 決まりは以前と同じ :
 *   - 触れた物体の速さが 0.5 以上で、最近（0.5 秒以内）触れていなければ、入・切を切り替える（do_switch）。触れたら毎回覚え直す
 *   - do_switch は 壊れていれば何もしない。入なら切に、切なら入にして、"turn-off"・"turn-on" のイベントを呼ぶ
 *   - turn_on・turn_off・do_break はイベントを呼ばない（ステージの game_object.action から呼ばれる）
 *   - 通常のスイッチは、入で "TurnOn" と "switch-on"、切で "TurnOff" と "switch-off"、壊れると "OnOff"（繰り返し）を鳴らす
 *
 * EntityManager::update() では何もしない（当たりの互換の層・ステージのコマンド・開発用の確かめが呼ぶ）
 */
class SwitchSystem : public core::ecs::System< SwitchComponent >
{
public:
	using EntityId		= core::ecs::EntityId;
	using State			= SwitchComponent::State;
	using EventHandler	= SwitchComponent::EventHandler;

private:
	core::sound::SoundManager* sound_manager_;		///< 音を鳴らす先（nullptr なら鳴らさない。テスト）

	SwitchComponent* find_switch( EntityId id );

	/// 通常のスイッチの見た目と音（以前の Switch::on_turn_on・on_turn_off・on_break）
	void play_feedback( EntityId id, const char_t* animation, bool loop, const char_t* sound );

	/// イベントの受け取り手を加えた順に呼ぶ（以前の BaseSwitch::on_event）
	void call_event( EntityId id, const string_t& name );

public:
	explicit SwitchSystem( core::sound::SoundManager* sound_manager );

	void update() override { }		///< 何もしない（上の説明のとおり）

	/**
	 * 物体が触れた（以前の BaseSwitch::on_collide_with( Player* )・on_collide_with( Stone* )）
	 *
	 * @param id スイッチ
	 * @param toucher_id 触れた物体（間引きに使う）
	 * @param velocity 触れた物体の速度
	 */
	void on_touch( EntityId id, EntityId toucher_id, const core::Vector& velocity );

	/// 入・切を切り替え、イベントを呼ぶ（変わったら true）
	bool do_switch( EntityId id );

	/// 壊す（以後は切り替わらない。壊れたら true）
	bool do_break( EntityId id );

	/// 入にする（イベントは呼ばない。変わったら true）
	bool turn_on( EntityId id );

	/// 切にする（イベントは呼ばない。変わったら true）
	bool turn_off( EntityId id );

	/// イベントの受け取り手を加える（スイッチでなければ false）
	bool add_event_handler( EntityId id, const string_t& name, const EventHandler& handler );

	/// 状態（スイッチでなければ std::nullopt）
	std::optional< State > get_state( EntityId id ) const;
};

} // namespace blue_sky::ecs
