#include "AsyncLoader.h"

#include <windows.h>
#include <objbase.h>

namespace core
{

AsyncLoader::AsyncLoader()
	: worker_( [this] { run(); } )
{

}

AsyncLoader::~AsyncLoader()
{
	{
		std::lock_guard< std::mutex > lock( mutex_ );
		stop_ = true;
		jobs_.clear();		// 残りの仕事は捨てる
	}

	cv_.notify_all();

	if ( worker_.joinable() )
	{
		worker_.join();
	}
}

void AsyncLoader::run()
{
	// WIC（画像の展開）などが COM を使うので、作業スレッドでも初期化する
	const HRESULT com = CoInitializeEx( nullptr, COINIT_MULTITHREADED );

	for ( ;; )
	{
		Job job;

		{
			std::unique_lock< std::mutex > lock( mutex_ );
			cv_.wait( lock, [this] { return stop_ || ! jobs_.empty(); } );

			if ( stop_ )
			{
				break;
			}

			job = std::move( jobs_.front() );
			jobs_.pop_front();
			running_++;
		}

		std::function< void() > on_done;

		try
		{
			on_done = job();
		}
		catch ( ... )
		{
			// 仕事の中の例外で作業スレッドを止めない。失敗は仕事の側で結果として返すこと
			on_done = nullptr;
		}

		{
			std::lock_guard< std::mutex > lock( mutex_ );
			running_--;

			if ( on_done )
			{
				completed_.push_back( std::move( on_done ) );
			}
		}

		cv_.notify_all();
	}

	if ( SUCCEEDED( com ) )
	{
		CoUninitialize();
	}
}

void AsyncLoader::enqueue_job( Job job )
{
	{
		std::lock_guard< std::mutex > lock( mutex_ );
		jobs_.push_back( std::move( job ) );
	}

	cv_.notify_all();
}

size_t AsyncLoader::dispatch_completed()
{
	std::deque< std::function< void() > > done;

	{
		std::lock_guard< std::mutex > lock( mutex_ );
		done.swap( completed_ );
	}

	for ( auto& f : done )
	{
		f();
	}

	return done.size();
}

size_t AsyncLoader::get_pending_count() const
{
	std::lock_guard< std::mutex > lock( mutex_ );
	return jobs_.size() + running_ + completed_.size();
}

size_t AsyncLoader::get_ready_count() const
{
	std::lock_guard< std::mutex > lock( mutex_ );
	return completed_.size();
}

void AsyncLoader::wait_all()
{
	for ( ;; )
	{
		{
			std::unique_lock< std::mutex > lock( mutex_ );
			cv_.wait( lock, [this] { return stop_ || ( jobs_.empty() && running_ == 0 ); } );
		}

		// 終わったときの処理が新しい仕事を積むことがあるので、積まれなくなるまで繰り返す
		if ( dispatch_completed() == 0 && get_pending_count() == 0 )
		{
			return;
		}
	}
}

} // namespace core
