// #pragma comment ( lib, "imgui.lib" )

#pragma comment( lib, "LibOVR.lib" )

// #pragma comment ( lib, "libfbxsdk.lib" )
// #ifdef _DEBUG
// #pragma comment ( lib, "libfbxsdk-md.lib" )
// #pragma comment ( lib, "libxml2-md.lib" )
// #pragma comment ( lib, "zlib-md.lib" )
// #else
#pragma comment ( lib, "libfbxsdk-mt.lib" )
#pragma comment ( lib, "libxml2-mt.lib" )
#pragma comment ( lib, "zlib-mt.lib" )
// #endif

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

// #pragma comment( lib, "libremidi.lib" )

#pragma comment( lib, "lua.lib" )

#pragma comment( lib, "common.lib" )
#pragma comment( lib, "game.lib" )
#pragma comment( lib, "win.lib" )
