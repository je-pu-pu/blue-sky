#pragma once

#include <cstddef>

namespace core::graphics
{

/**
 * 定数バッファの置き場所と更新のしかた（作るときに決める）
 */
enum class ConstantBufferUsage
{
	DEFAULT,	///< GPU だけが読み書きする。更新はコピーを頼む（D3D11 : DEFAULT + UpdateSubresource）。たまにしか変えないもの向け。以前はすべてこれだった
	DYNAMIC,	///< CPU から書き込む。更新は前の中身を捨てて書き直す（D3D11 : DYNAMIC + Map(WRITE_DISCARD)）。毎フレーム・毎描画で変えるもの向け
};

/**
 * シェーダーから利用する定数バッファ
 *
 */
class ConstantBuffer
{
public:
	using Usage = ConstantBufferUsage;

	/// 定数バッファの大きさは 16 バイトの倍数でなければならないので、切り上げる
	static constexpr size_t get_aligned_size( size_t size ) { return ( size + 15 ) / 16 * 16; }

private:
	Usage usage_;

public:
	explicit ConstantBuffer( Usage usage = Usage::DEFAULT ) : usage_( usage ) { }
	virtual ~ConstantBuffer() = default;
	ConstantBuffer( const ConstantBuffer& ) = delete;
	ConstantBuffer& operator = ( const ConstantBuffer& ) = delete;

	Usage get_usage() const { return usage_; }

	virtual void update( const void* data ) const = 0;

	virtual void bind_to_vs() const = 0;
	virtual void bind_to_hs() const = 0;
	virtual void bind_to_ds() const = 0;
	virtual void bind_to_cs() const = 0;
	virtual void bind_to_gs() const = 0;
	virtual void bind_to_ps() const = 0;

	virtual void bind_to_all() const
	{
		bind_to_vs();
		bind_to_hs();
		bind_to_ds();
		bind_to_cs();
		bind_to_gs();
		bind_to_ps();
	}
};

} // namespace core::graphics
