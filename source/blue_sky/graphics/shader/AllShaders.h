#pragma once

/**
 * GraphicsManager が既定で作るシェーダーのヘッダをまとめて読み込む
 *
 * 以前は GraphicsManager.cpp に並べていた。既定のシェーダーを足すときはここに足す。
 */

#include <blue_sky/graphics/shader/NullShader.h>
#include <blue_sky/graphics/shader/FlatShader.h>
#include <blue_sky/graphics/shader/LitShader.h>
#include <blue_sky/graphics/shader/LitInstancedShader.h>
#include <blue_sky/graphics/shader/FaderShader.h>
#include <blue_sky/graphics/shader/MatcapShader.h>
#include <blue_sky/graphics/shader/ShadowMapShader.h>
#include <blue_sky/graphics/shader/TessellationMatcapShader.h>
#include <blue_sky/graphics/shader/UnicolorShader.h>
#include <blue_sky/graphics/shader/VelocityShader.h>
#include <blue_sky/graphics/shader/DebugShadowMapTextureShader.h>
#include <blue_sky/graphics/shader/post_effect/DefaultShader.h>
#include <blue_sky/graphics/shader/post_effect/HandDrawingShader.h>
#include <blue_sky/graphics/shader/post_effect/RecolorShader.h>
#include <blue_sky/graphics/shader/post_effect/NoiseShader.h>
