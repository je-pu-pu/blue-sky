#pragma once

#include <core/type.h>

#include <functional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace game
{
	class Config;
}

namespace core::setting
{

/**
 * 設定値の反映方式
 */
enum class ApplyMode
{
	IMMEDIATE,	///< 変更した瞬間に反映する（音量・感度など）。戻るときに自動で確定する
	DEFERRED	///< 「適用」まで反映しない（解像度・フルスクリーンなど）
};

/**
 * ゲーム設定のストア
 *
 * 各項目について「既定値」「確定値（保存される値）」「保留値（画面で編集中の値）」を持つ。
 * UI は保留値を読み書きし、適用・元に戻す・既定値に戻す・ロールバックをこのクラスに任せる。
 *
 * - 値の型は bool / int / float / 文字列。登録した型に自動で揃える（int を float 項目に入れても可）
 * - float は step に量子化して保存し、比較は誤差を許容する
 * - 反映関数（ApplyFn）は IMMEDIATE なら set() 時、DEFERRED なら apply_deferred() 時に呼ばれる
 * - 複数キーをまとめて 1 回だけ反映したい場合（解像度の幅と高さ等）はグループを登録する
 */
class SettingStore
{
public:
	using Value = std::variant< bool, int, float_t, string_t >;
	using ApplyFn = std::function< void( const Value& ) >;
	using OnSetHook = std::function< void( SettingStore&, const string_t& key, const Value& old_value ) >;
	using Snapshot = std::unordered_map< string_t, Value >;

private:
	struct Entry
	{
		string_t key;
		Value default_value;
		Value committed;
		Value pending;
		float_t min = 0.f;
		float_t max = 0.f;
		float_t step = 0.f;
		bool has_range = false;
		ApplyMode mode = ApplyMode::IMMEDIATE;
		ApplyFn apply;
		OnSetHook on_set;
		int group = -1;		///< 所属グループ（-1 = なし）
	};

	struct Group
	{
		std::vector< string_t > keys;
		std::function< void() > apply;
	};

	std::vector< Entry > entries_;
	std::unordered_map< string_t, size_t > index_;
	std::vector< Group > groups_;

	bool in_hook_ = false;
	uint_t change_count_ = 0;

	Snapshot trial_values_;		///< 試用中の項目と、保存に使う元の値

	Entry* find( const string_t& key );
	const Entry* find( const string_t& key ) const;

	Entry& add_entry( const string_t& key, const Value& def, ApplyMode mode, ApplyFn apply );

	/// 登録型に変換し、範囲・量子化を適用した値を返す
	Value normalize( const Entry&, const Value& ) const;

	bool equals( const Entry&, const Value&, const Value& ) const;

	/// 変化した項目の反映関数・グループ反映関数を呼ぶ（グループは 1 回だけ）
	void apply_changed( const std::vector< size_t >& changed );

	static string_t to_text( const Value& );
	static bool parse_text( const Value& type_sample, const string_t& text, Value& out );

public:
	// 登録
	void add_bool( const string_t& key, bool def, ApplyMode mode, ApplyFn apply = {} );
	void add_int( const string_t& key, int def, int min, int max, ApplyMode mode, ApplyFn apply = {} );
	void add_float( const string_t& key, float_t def, float_t min, float_t max, float_t step, ApplyMode mode, ApplyFn apply = {} );
	void add_string( const string_t& key, const string_t& def, ApplyMode mode, ApplyFn apply = {} );

	/// DEFERRED の複数キーをまとめて反映する関数を登録する。グループ内の個別 ApplyFn は呼ばれなくなる
	void add_apply_group( const std::vector< string_t >& keys, std::function< void() > apply );

	/// set() の直後に呼ばれるフック（関連する他の項目の調整用）。フック内の set() ではフックは呼ばれない
	void set_on_set( const string_t& key, OnSetHook hook );

	bool has( const string_t& key ) const { return find( key ) != nullptr; }
	size_t size() const { return entries_.size(); }

	// 値（保留値）
	Value get_value( const string_t& key ) const;

	template< typename T > T get( const string_t& key ) const
	{
		const Value v = get_value( key );

		if ( const T* p = std::get_if< T >( & v ) )
		{
			return *p;
		}

		return T();
	}

	Value get_committed( const string_t& key ) const;
	Value get_default( const string_t& key ) const;

	/// 保留値を設定する。範囲・量子化を適用し、IMMEDIATE なら反映関数を呼ぶ
	void set( const string_t& key, const Value& value );
	void set( const string_t& key, const char_t* value ) { set( key, Value( string_t( value ) ) ); }

	// 状態
	bool has_pending_changes() const;		///< 保留値と確定値が違う項目があるか
	bool has_pending_deferred() const;		///< DEFERRED の未適用変更があるか
	bool is_pending( const string_t& key ) const;

	// 操作
	void commit_immediate();			///< IMMEDIATE の保留値を確定する
	bool apply_deferred();				///< DEFERRED の保留値を確定して反映する。変化があれば true
	void revert();						///< 全項目の保留値を確定値に戻す（IMMEDIATE で変わっていた分は反映し直す）
	void revert_deferred();				///< DEFERRED の保留値だけ確定値に戻す
	void reset_to_defaults();			///< 保留値を既定値にする（IMMEDIATE は反映する。確定はしない）

	Snapshot snapshot() const;			///< 確定値の写し
	void rollback( const Snapshot&, const std::vector< string_t >& keys );	///< 指定キーの確定値・保留値を写しに戻して反映する（試用中なら試用も終える）

	/**
	 * 試用を始める（表示モードの変更のように、ユーザーが「維持」を選ぶまで確定しない変更）
	 *
	 * 試用中は、save() が keys について確定値ではなく before の値を書く。
	 * 確認の途中で強制終了・クラッシュしても、設定ファイルには元の（映っていた）値が残る。
	 * 維持なら end_trial()、取り消しなら rollback() で終える。
	 */
	void begin_trial( const Snapshot& before, const std::vector< string_t >& keys );
	void end_trial() { trial_values_.clear(); }
	bool in_trial() const { return ! trial_values_.empty(); }

	/// 外部（ホットキー等）で変わった実際の値を、確定値・保留値の両方に取り込む。反映関数は呼ばない
	void sync_from_runtime( const string_t& key, const Value& value );

	// 永続化
	void load( const game::Config& );	///< 読めない値・範囲外は既定値／クランプ
	void save( game::Config& ) const;	///< 確定値を書く

	/// 起動時: グループに属さない項目の反映関数を確定値で呼ぶ
	void apply_all_individual();

	/// 実際に反映される値が変わるたびに増える（シーン側の再読み込み判定用）
	uint_t get_change_count() const { return change_count_; }

}; // class SettingStore

} // namespace core::setting
