#pragma once

namespace blue_sky
{

/**
 * 落ちたとき（アクセス違反などの構造化例外）に、例外の種類と呼び出し履歴を log/crash.log に書く
 *
 * 以前は何も残らず終了し、どこで落ちたのか分からなかった。
 * 呼び出し履歴は exe と同じ場所の PDB から関数名・行番号を引く（無ければアドレスだけ）。
 * WinMain の最初に一度だけ呼ぶ。
 */
void install_crash_report();

} // namespace blue_sky
