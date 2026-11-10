#pragma once

namespace core::ecs { class Entity; }
namespace core::sound { class SoundManager; }

/**
 * 物体どうしが当たったときの決まり（部品の組み合わせで決める）
 *
 * 以前は ActiveObjectManager（当たりの互換の層）の on_collide_entity_objects・on_collide_with_player・on_collide_with_entity_object で、
 * さらに前は種類ごとのクラスの on_collide_with の二重ディスパッチだった。段階 6 でプレイヤーも ECS だけの物体になり、
 * ActiveObject との組が無くなったので、部品の組み合わせだけで決める関数としてここへ移した。
 * 当たった組を集めて渡すのは ecs::CollisionEventSystem、渡された組をここへ回すのは ObjectUpdater
 */
namespace blue_sky::ecs::contact_rules
{

/**
 * 2 つの物体が当たった（1 組につき 1 回。組の向きによらない）
 *
 * - プレイヤー（PlayerBehaviorComponent）と物体 : on_collide_with_player
 * - 石（ThrowableComponent）と次の物（それ以外の組は何もしない。以前と同じ）:
 *   - 風船（BalloonHoldComponent）: 割れる（以前の Balloon::on_collide_with_stone）
 *   - 敵（RobotBehaviorComponent）: 立つ・回る間に、こちらへ飛んでくる石なら止まる（以前の Robot::on_collide_with_stone）
 *   - スイッチ（SwitchComponent）: 石の速さで入・切が変わる（以前の BaseSwitch::on_collide_with( Stone* )。空間スイッチは石では変わらない）
 *
 * @param sound_manager 梯子が触れた音を鳴らす先（nullptr なら鳴らさない。テスト）
 */
void on_collide( core::ecs::Entity& a, core::ecs::Entity& b, core::sound::SoundManager* sound_manager = nullptr );

/**
 * プレイヤーと物体が当たった
 *
 * 物体の部品の組み合わせで、プレイヤーの System（PlayerBehaviorSystem の on_touch_*）に知らせる。順と回数は、以前プレイヤーが ActiveObject だったときと同じ
 * （動かない物 → 拾う物 → 石 → スイッチ → 風船 → 敵 → 梯子。1 組につき 1 回）。以前は GameObject の仮想関数（on_pick_up・on_collide_with_balloon など）をプレイヤーが実装していた
 */
void on_collide_with_player( core::ecs::Entity& player, core::ecs::Entity& entity, core::sound::SoundManager* sound_manager = nullptr );

} // namespace blue_sky::ecs::contact_rules
