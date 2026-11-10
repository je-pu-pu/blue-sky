#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

namespace core
{

/**
 * 重い読み込み（ファイルの読み込み・画像の展開など CPU の仕事）を別のスレッドで行い、
 * 終わったら結果の処理をメインスレッドで行う（設計上の問題 B23）
 *
 * - enqueue( 仕事, 終わったときの処理 ) : 仕事は作業スレッドで、終わったときの処理はメインスレッドの dispatch_completed で呼ぶ
 * - GPU のリソースの作成・シーンの組み立てなど、メインスレッドでしかできないことは「終わったときの処理」に書く
 * - 作業スレッドは COM（MTA）を初期化してから仕事をする（WIC の画像の展開が使う）
 * - 破棄すると、残りの仕事を捨てて作業スレッドを止める（実行中の 1 件は終わるまで待つ）
 *
 * まだ使う側が無い : これを使うのは GraphicsManager の start_texture_load_async（テクスチャの別スレッド読み込み）だけで、
 * それを呼ぶ load_named_texture_premultiplied_async もまだどこからも呼ばれていない。シーンの読み込みは同期のまま
 * （「読み込み中」を 1 回描いてから読む）。非同期の読み込みを使うのは後続の作業。仕組みはテスト（AsyncLoaderTest）付きで残す。
 */
class AsyncLoader
{
public:
	/// 作業スレッドで行う仕事。終わったらメインスレッドで呼ぶ処理を返す
	using Job = std::function< std::function< void() >() >;

private:
	mutable std::mutex mutex_;
	std::condition_variable cv_;
	std::deque< Job > jobs_;
	std::deque< std::function< void() > > completed_;
	size_t running_ = 0;
	bool stop_ = false;
	std::thread worker_;

	void run();

public:
	AsyncLoader();
	~AsyncLoader();

	AsyncLoader( const AsyncLoader& ) = delete;
	AsyncLoader& operator = ( const AsyncLoader& ) = delete;

	/// 仕事を積む（どのスレッドからでもよい）
	void enqueue_job( Job job );

	/**
	 * 仕事を積む（結果の型付き）
	 *
	 * @param work 作業スレッドで行う仕事（結果を返す）
	 * @param on_done メインスレッドで結果を受け取る処理
	 */
	template< typename Result >
	void enqueue( std::function< Result() > work, std::function< void( Result ) > on_done )
	{
		enqueue_job( [work = std::move( work ), on_done = std::move( on_done )] () -> std::function< void() >
		{
			auto result = std::make_shared< Result >( work() );
			return [on_done, result] { on_done( std::move( * result ) ); };
		} );
	}

	/// 終わった仕事の処理を呼ぶ（メインスレッドで毎フレーム呼ぶ）。呼んだ数を返す
	size_t dispatch_completed();

	/// まだ終わっていない仕事の数（積んだもの・実行中・メインスレッドでの処理待ちを含む）
	size_t get_pending_count() const;

	/// 作業スレッドで終わり、メインスレッドでの処理（dispatch_completed）を待っている数
	size_t get_ready_count() const;

	/// すべての仕事が終わり、終わったときの処理も呼び終えるまで待つ（メインスレッドから呼ぶ。読み込み画面などで使う）
	void wait_all();

}; // class AsyncLoader

} // namespace core
