#!/usr/bin/env bash
# blue-sky x64 依存(OSS)ライブラリを取得・ビルドし source/lib/x64 へ配置する。
# 詳細・ハマりどころは同ディレクトリ README.md を参照。
# 前提: VS18 / VS同梱cmake / 7-Zip / git / curl。FBX SDK は手動（README ②）。
set -euo pipefail

# --- パス解決 -------------------------------------------------------------
CM="$(ls "/c/Program Files/Microsoft Visual Studio/"*/*/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe 2>/dev/null | head -1)"
SZ="/c/Program Files/7-Zip/7z.exe"
VCVARS="$(ls "/c/Program Files/Microsoft Visual Studio/"*/*/VC/Auxiliary/Build/vcvars64.bat 2>/dev/null | head -1)"
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DST="$REPO/source/lib/x64"
WORK="${WORK:-$REPO/.x64deps-work}"
RT='-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug> -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_POLICY_VERSION_MINIMUM=3.5'
mkdir -p "$DST/debug" "$DST/release" "$WORK"; cd "$WORK"
echo "cmake=$CM"; echo "work=$WORK"; echo "dst=$DST"

cfgbuild() { "$CM" --build "$1" --config Debug -j8 >/dev/null && "$CM" --build "$1" --config Release -j8 >/dev/null; }

# --- libogg 1.3.5 ---------------------------------------------------------
[ -d ogg ] || git clone --depth1 -b v1.3.5 https://github.com/xiph/ogg.git ogg
"$CM" -S ogg -B ogg/b -A x64 -DBUILD_SHARED_LIBS=OFF -DINSTALL_DOCS=OFF $RT >/dev/null
cfgbuild ogg/b
cp ogg/b/Debug/ogg.lib "$DST/debug/libogg.lib"; cp ogg/b/Release/ogg.lib "$DST/release/libogg.lib"

# --- libvorbis 1.3.7 ------------------------------------------------------
[ -d vorbis ] || git clone --depth1 -b v1.3.7 https://github.com/xiph/vorbis.git vorbis
"$CM" -S vorbis -B vorbis/b -A x64 -DBUILD_SHARED_LIBS=OFF $RT \
  -DOGG_INCLUDE_DIR="$WORK/ogg/include;$WORK/ogg/b/include" -DOGG_LIBRARY="$WORK/ogg/b/Debug/ogg.lib" >/dev/null
cfgbuild vorbis/b
cp vorbis/b/lib/Debug/vorbis.lib     "$DST/debug/libvorbis_static.lib"
cp vorbis/b/lib/Debug/vorbisfile.lib "$DST/debug/libvorbisfile_static.lib"
cp vorbis/b/lib/Release/vorbis.lib     "$DST/release/libvorbis_static.lib"
cp vorbis/b/lib/Release/vorbisfile.lib "$DST/release/libvorbisfile_static.lib"

# --- Bullet 3.25 ----------------------------------------------------------
[ -d bullet3 ] || git clone --depth1 -b 3.25 https://github.com/bulletphysics/bullet3.git bullet3
"$CM" -S bullet3 -B bullet3/b -A x64 -DBUILD_SHARED_LIBS=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF $RT \
  -DBUILD_BULLET2_DEMOS=OFF -DBUILD_CPU_DEMOS=OFF -DBUILD_OPENGL3_DEMOS=OFF -DBUILD_UNIT_TESTS=OFF -DBUILD_EXTRAS=OFF -DBUILD_BULLET3=OFF >/dev/null
for c in Debug Release; do "$CM" --build bullet3/b --config $c --target BulletCollision BulletDynamics LinearMath -j8 >/dev/null; done
for n in BulletCollision BulletDynamics LinearMath; do
  cp "bullet3/b/lib/Debug/${n}_Debug.lib" "$DST/debug/${n}_vs2010_debug.lib"
  cp "bullet3/b/lib/Release/${n}.lib"     "$DST/release/${n}_vs2010.lib"
done

# --- PortAudio 19.7.0 (WDM-KS 無効) ---------------------------------------
[ -d portaudio ] || git clone --depth1 -b v19.7.0 https://github.com/PortAudio/portaudio.git portaudio
"$CM" -S portaudio -B portaudio/b -A x64 -DBUILD_SHARED_LIBS=OFF -DPA_BUILD_SHARED=OFF -DPA_BUILD_STATIC=ON \
  -DPA_DLL_LINK_WITH_STATIC_RUNTIME=ON -DPA_USE_WDMKS=OFF -DPA_USE_WDMKS_DEVICE_INFO=OFF $RT >/dev/null
cfgbuild portaudio/b
cp portaudio/b/Debug/portaudio_static_x64.lib   "$DST/debug/portaudio_static.lib"
cp portaudio/b/Release/portaudio_static_x64.lib "$DST/release/portaudio_static.lib"

# --- Effects11 (FX11) -----------------------------------------------------
[ -d FX11 ] || git clone --depth1 https://github.com/microsoft/FX11.git FX11
"$CM" -S FX11 -B FX11/b -A x64 $RT >/dev/null
cfgbuild FX11/b
cp FX11/b/lib/Debug/Effects11.lib   "$DST/debug/effects11d.lib"
cp FX11/b/lib/Release/Effects11.lib "$DST/release/effects11.lib"

# --- Lua 5.4.3 (静的CRT。ヘッダが 5.4.3) ----------------------------------
[ -d lua-5.4.3 ] || { curl -sL https://www.lua.org/ftp/lua-5.4.3.tar.gz -o lua.tgz; "$SZ" x lua.tgz -y >/dev/null; "$SZ" x lua-5.4.3.tar -y >/dev/null; }
mkdir -p luab
cat > luab/CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.15)
project(lualib C)
set(LSRC "$WORK/lua-5.4.3/src")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded\$<\$<CONFIG:Debug>:Debug>")
file(GLOB C "\${LSRC}/*.c")
list(REMOVE_ITEM C "\${LSRC}/lua.c" "\${LSRC}/luac.c")
add_library(lua STATIC \${C})
target_include_directories(lua PRIVATE \${LSRC})
EOF
"$CM" -S luab -B luab/b -A x64 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW >/dev/null
cfgbuild luab/b
cp luab/b/Debug/lua.lib "$DST/debug/lua.lib"; cp luab/b/Release/lua.lib "$DST/release/lua.lib"

# --- boost 1.90.0 (b2 は Device Guard で不可 → 自作 CMake) -----------------
[ -d boost_1_90_0 ] || { curl -sL https://archives.boost.io/release/1.90.0/source/boost_1_90_0.7z -o boost.7z; "$SZ" x boost.7z -y >/dev/null; }
mkdir -p boostb
cat > boostb/CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.15)
project(boostlibs CXX)
set(CMAKE_CXX_STANDARD 20)
set(BR "$WORK/boost_1_90_0")
include_directories(\${BR})
add_definitions(-DBOOST_ALL_NO_LIB -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded\$<\$<CONFIG:Debug>:Debug>")
foreach(lib filesystem chrono timer regex atomic)
  file(GLOB SRC "\${BR}/libs/\${lib}/src/*.cpp")
  if(SRC)
    add_library(boost_\${lib} STATIC \${SRC})
  endif()
endforeach()
EOF
"$CM" -S boostb -B boostb/b -A x64 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW >/dev/null
cfgbuild boostb/b
for lib in filesystem chrono timer regex atomic; do
  cp "boostb/b/Debug/boost_${lib}.lib"   "$DST/debug/libboost_${lib}-vc143-mt-sgd-x64-1_90.lib"
  cp "boostb/b/Release/boost_${lib}.lib" "$DST/release/libboost_${lib}-vc143-mt-s-x64-1_90.lib"
done
# boost ヘッダ（git 管理外）を配置
mkdir -p "$REPO/source/lib/include/boost"
"$CM" -E copy_directory "$WORK/boost_1_90_0/boost" "$REPO/source/lib/include/boost"

echo "=== 完了。FBX SDK は README ② を手動で実施してください ==="
