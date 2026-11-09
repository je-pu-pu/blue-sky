#pragma once

#include <common/exception.h>

#include <string>

namespace core::graphics
{

/**
 * GPU のデバイスが失われた（ドライバの更新・GPU のタイムアウト・GPU の切り替えなど）
 *
 * GPU のリソースは各所が持っていて、その場で全部を作り直すことはできない。
 * WinMain がこれを受け取り、ゲームを起動し直して復旧する（設計上の問題 A9）。
 */
class DeviceLostException : public common::exception< std::string >
{
public:
	using common::exception< std::string >::exception;
};

} // namespace core::graphics
