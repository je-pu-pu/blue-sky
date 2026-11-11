#pragma once

#include <core/type.h>
#include <core/setting/SettingKey.h>

#include <functional>
#include <type_traits>
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
	DEFERRED,	///< 「適用」まで反映しない（解像度・フルスクリーンなど）
	ON_RELEASE	///< 操作を終えたら反映する（スライダーをドラッグし終えたら。apply_pending で反映）。確定の扱いは IMMEDIATE と同じ
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

	/// 使い方の誤り（未登録のキー・型の違い）を知らせる先。既定は何もしない（ログに出すのはアプリ側で設定する）
	using ErrorHandler = std::function< void( const string_t& message ) >;

	/// 範囲（int / float の項目）
	struct Range
	{
		float_t min = 0.f;
		float_t max = 0.f;
		float_t step = 0.f;		///< 0 なら刻み無し
	};

	/// 画面に出すための情報（ラベル・説明の文字列キー）
	struct Info
	{
		string_t label_key;
		string_t description_key;
	};

	/// 保存するファイルの形式の版（項目の意味を変えたときに上げ、読み込み側で移行する）
	static constexpr const char_t* VERSION_KEY = "settings.version";

	/// 値が変わったときに呼ばれる関数（引数は反映される値）
	using Listener = std::function< void( const Value& ) >;

	/**
	 * 購読（subscribe の戻り値）。破棄すると購読をやめる
	 *
	 * シーンなどストアより先に破棄されるものがメンバーに持つ。ストアは購読より長生きすること。
	 */
	class Subscription
	{
		SettingStore* store_ = nullptr;
		uint_t id_ = 0;

	public:
		Subscription() = default;
		Subscription( SettingStore* store, uint_t id ) : store_( store ), id_( id ) { }
		~Subscription() { reset(); }

		Subscription( const Subscription& ) = delete;
		Subscription& operator = ( const Subscription& ) = delete;

		Subscription( Subscription&& o ) noexcept : store_( o.store_ ), id_( o.id_ ) { o.store_ = nullptr; }
		Subscription& operator = ( Subscription&& o ) noexcept
		{
			if ( this != & o )
			{
				reset();
				store_ = o.store_;
				id_ = o.id_;
				o.store_ = nullptr;
			}
			return *this;
		}

		void reset();
	};

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
		bool apply_dirty = false;		///< ON_RELEASE : set したがまだ反映していない
		int group = -1;		///< 所属グループ（-1 = なし）
		Info info;
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

	std::function< void( SettingStore& ) > normalizer_;
	bool in_normalizer_ = false;

	/// 値を整える関数を呼ぶ（入れ子では呼ばない）
	void run_normalizer();
	uint_t change_count_ = 0;

	Snapshot trial_values_;		///< 試用中の項目と、保存に使う元の値

	ErrorHandler error_handler_;

	struct ListenerEntry
	{
		uint_t id;
		string_t key;
		Listener listener;
	};

	std::vector< ListenerEntry > listeners_;
	uint_t next_listener_id_ = 1;

	/// 反映される値が変わった項目の購読者に知らせる
	void notify( const Entry& e ) const;

	/// 使い方の誤りを知らせる
	void report( const string_t& message ) const;

	/// 項目を探す。無ければ（呼び出し元の名前付きで）誤りを知らせる
	Entry* find_or_report( const string_t& key, const char_t* caller );
	const Entry* find_or_report( const string_t& key, const char_t* caller ) const;

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

	/**
	 * 値を整える関数（不正な値・重複したキー割り当ての修正など）を登録する
	 *
	 * set・load・revert・revert_deferred・reset_to_defaults・rollback のあとに必ず呼ぶ。
	 * 以前は set_on_set のフックが set() のときしか動かず、読み込み・元に戻すの経路では検査されなかった。
	 * 中で set() を呼んでよい（入れ子では呼ばない）
	 */
	void set_normalizer( std::function< void( SettingStore& ) > normalizer ) { normalizer_ = std::move( normalizer ); }

	/// 操作を終えたら反映する項目（ON_RELEASE）の、まだ反映していない値を反映する。それ以外の項目では何もしない
	void apply_pending( const string_t& key );

	/// 画面に出すための情報を付ける（ラベル・説明の文字列キー）
	void set_info( const string_t& key, const string_t& label_key, const string_t& description_key );
	Info get_info( const string_t& key ) const;

	/// 範囲（int / float の項目）。範囲の無い項目なら false
	bool get_range( const string_t& key, Range& range ) const;

	bool has( const string_t& key ) const { return find( key ) != nullptr; }
	size_t size() const { return entries_.size(); }

	/// 使い方の誤り（未登録のキー・型の違い）を知らせる先を設定する
	void set_error_handler( ErrorHandler handler ) { error_handler_ = std::move( handler ); }

	/**
	 * 項目の値が変わったら（反映されるとき）知らせてもらう
	 *
	 * 以前はシーンが全体の変更回数（get_change_count）を毎フレーム調べていて、
	 * FOV を反映していたのは GamePlayScene だけだった。購読すれば、変わった項目だけが届く。
	 * 戻り値を持っている間だけ有効。
	 */
	[[nodiscard]] Subscription subscribe( const string_t& key, Listener listener );

	/// 購読をやめる（Subscription が自動で呼ぶ）
	void unsubscribe( uint_t id );

	// 値（保留値）
	Value get_value( const string_t& key ) const;

	/// 名前で読む。未登録のキー・型の違いは誤りを知らせて T() を返す。新しいコードは SettingKey 版を使う
	template< typename T > T get( const string_t& key ) const
	{
		const Entry* e = find_or_report( key, "get" );

		if ( ! e )
		{
			return T();
		}

		if ( const T* p = std::get_if< T >( & e->pending ) )
		{
			return *p;
		}

		report( "設定 " + key + " を登録と違う型で読もうとしました" );
		return T();
	}

	/// 型付きのキーで読む（型の書き間違いはコンパイル時に分かる）
	template< typename T > T get( const SettingKey< T >& key ) const { return get< T >( key.str() ); }

	/// 型付きのキーで書く（値の型はキーから決める。float の項目に 0.5 と書いても double と食い違わない）
	template< typename T > void set( const SettingKey< T >& key, const std::type_identity_t< T >& value ) { set( key.str(), Value( value ) ); }

	Value get_committed( const string_t& key ) const;
	Value get_default( const string_t& key ) const;

	/// 確定値を型付きのキーで読む
	template< typename T > T get_committed( const SettingKey< T >& key ) const
	{
		const Value v = get_committed( key.str() );
		const T* p = std::get_if< T >( & v );
		return p ? *p : T();
	}

	/// 保留値を設定する。範囲・量子化を適用し、IMMEDIATE なら反映関数を呼ぶ
	void set( const string_t& key, const Value& value );
	void set( const string_t& key, const char_t* value ) { set( key, Value( string_t( value ) ) ); }
	void set( const SettingKey< string_t >& key, const char_t* value ) { set( key.str(), Value( string_t( value ) ) ); }

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

	/**
	 * 確定値を書く
	 *
	 * 既定値と同じ項目は書かない（設定から消す）。コード側で既定値を変えたとき、既存のユーザーにも届くように。
	 * 試用中の項目は元の値を書く。VERSION_KEY に version を書く。
	 */
	void save( game::Config&, int version = 1 ) const;

	/// 起動時: グループに属さない項目の反映関数を確定値で呼ぶ
	void apply_all_individual();

	/// 実際に反映される値が変わるたびに増える（シーン側の再読み込み判定用）
	uint_t get_change_count() const { return change_count_; }

}; // class SettingStore

} // namespace core::setting
