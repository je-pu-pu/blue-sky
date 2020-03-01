#pragma once

namespace core::graphics
{

/**
 * シェーダーから利用する定数バッファ
 *
 */
class ConstantBuffer
{
public:
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
