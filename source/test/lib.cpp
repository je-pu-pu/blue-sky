#include "pch.h"

#pragma comment( lib, "LibOVR.lib" )

#pragma comment ( lib, "libfbxsdk-mt.lib" )
// #pragma comment ( lib, "libxml2-mt.lib" ) /// libxml2-mt.lib を #pragma comment でリンクすると blue-sky のテストが Visual Studio のテストエクスプローラーに表示されなくなる不具合を回避するため、プロジェクトの設定でリンクを行う。
#pragma comment ( lib, "zlib-mt.lib" )

#pragma comment ( lib, "libogg.lib" )
#pragma comment ( lib, "libvorbis_static.lib" )
#pragma comment ( lib, "libvorbisfile_static.lib" )

#ifdef _DEBUG
#pragma comment( lib, "BulletCollision_vs2010_debug.lib" )
#pragma comment( lib, "BulletDynamics_vs2010_debug.lib" )
#pragma comment( lib, "LinearMath_vs2010_debug.lib" )
#else
#pragma comment( lib, "BulletCollision_vs2010.lib" )
#pragma comment( lib, "BulletDynamics_vs2010.lib" )
#pragma comment( lib, "LinearMath_vs2010.lib" )
#endif 

#pragma comment( lib, "portaudio_static.lib" )

#pragma comment( lib, "lua.lib" )

#pragma comment( lib, "common.lib" )
#pragma comment( lib, "game.lib" )
#pragma comment( lib, "win.lib" )
