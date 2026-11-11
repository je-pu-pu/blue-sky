#include "OptionsController.h"

#include <core/setting/SettingStore.h>
#include <core/ui/Dialog.h>

#include <algorithm>
#include <cmath>

namespace blue_sky
{

using core::ui::DialogSpec;

void OptionsController::finish_and_close()
{
	deps_.store->commit_immediate();
	deps_.save();
	deps_.close();
}

void OptionsController::apply( bool close_after )
{
	auto& store = *deps_.store;

	const auto snapshot = store.snapshot();
	const bool display_changed = store.apply_deferred();

	store.commit_immediate();

	// 表示が変わったときは、維持が選ばれるまで表示系の項目は元の値で保存する
	// （確認の途中で強制終了・クラッシュしても、次回は映っていた設定で起動する）
	if ( display_changed )
	{
		store.begin_trial( snapshot, deps_.display_keys );
	}

	deps_.save();

	if ( ! display_changed )
	{
		if ( close_after )
		{
			deps_.close();
		}

		return;
	}

	// 表示が変わった : 時間内に「維持」しなければ元に戻す（映らなくなっても自動で復旧する）
	const float_t start = deps_.now();

	DialogSpec spec;
	spec.message_provider = [this, start]
	{
		const float_t elapsed = deps_.now() - start;
		const int remaining = std::max( 0, static_cast< int >( std::ceil( DISPLAY_CONFIRM_TIMEOUT - elapsed ) ) );
		return deps_.confirm_display_message( remaining );
	};
	spec.buttons =
	{
		{ "options.keep", [this, close_after]
			{
				deps_.store->end_trial();
				deps_.save();

				if ( close_after )
				{
					deps_.close();
				}
			} },
		{ "options.revert", [this, snapshot]
			{
				deps_.store->rollback( snapshot, deps_.display_keys );
				deps_.save();
			} },
	};
	spec.cancel_index = 1;		// キャンセル・時間切れは「元に戻す」
	spec.default_index = 0;
	spec.timeout_sec = DISPLAY_CONFIRM_TIMEOUT;

	show_dialog( *deps_.screen, std::move( spec ) );
}

void OptionsController::back()
{
	if ( ! deps_.store->has_pending_deferred() )
	{
		finish_and_close();
		return;
	}

	DialogSpec spec;
	spec.message = "options.confirm_unapplied";
	spec.buttons =
	{
		{ "options.apply_and_back", [this] { apply( true ); } },
		{ "options.discard_and_back", [this] { deps_.store->revert_deferred(); finish_and_close(); } },
		{ "common.cancel", nullptr },
	};
	spec.cancel_index = 2;
	spec.default_index = 0;

	show_dialog( *deps_.screen, std::move( spec ) );
}

void OptionsController::reset_to_defaults()
{
	DialogSpec spec;
	spec.message = "options.confirm_defaults";
	spec.buttons =
	{
		{ "common.yes", [this] { deps_.store->reset_to_defaults(); } },
		{ "common.no", nullptr },
	};
	spec.cancel_index = 1;
	spec.default_index = 1;

	show_dialog( *deps_.screen, std::move( spec ) );
}

} // namespace blue_sky
