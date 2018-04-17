/**
 * このファイルをインクルードすると、プログラム終了時に解放されていないメモリがどのソースのどの場所で確保されたものかが出力されるようになる。
 *
 * 注意 : このファイルをインクルードすると 他の場所で定義した operator new が使用されなくなるため、不具合が起こる。一時的なデバッグのためだけに使用する事。
 */

#ifndef WIN_MEMORY_LEAK_CHECK_H
#define WIN_MEMORY_LEAK_CHECK_H

#include "memory_leak_detection.h"

// new を置換する
#ifdef _DEBUG
#define new ::new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#endif

#endif // WIN_MEMORY_LEAK_DETECTION_H
