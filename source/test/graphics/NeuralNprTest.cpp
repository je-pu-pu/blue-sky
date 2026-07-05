#include "pch.h"

// ONNX Runtime + DirectML が x64 実機で動くかのスモークテスト。
// 採用モデル starry_boil_small.onnx（確率的スタイル化ネット、入力 image+noise）を
// DirectML EP でロードし、ダミー入力で推論できること＋速度を確認する。
#include <onnxruntime/onnxruntime_cxx_api.h>
#include <onnxruntime/dml_provider_factory.h>

#include <filesystem>
#include <vector>
#include <array>
#include <chrono>
#include <iostream>

namespace {

std::wstring model_path()
{
	// test/graphics/ から見て ../../../ = リポジトリルート
	auto p = std::filesystem::path( __FILE__ ).parent_path()
		/ "../../../tools/npr_offline/models/starry_boil_small.onnx";
	return p.lexically_normal().wstring();
}

} // namespace

TEST( NeuralNprTest, DmlInferenceSmoke )
{
	const std::wstring path = model_path();
	ASSERT_TRUE( std::filesystem::exists( path ) )
		<< "model not found (tools/npr_offline/models/starry_boil_small.onnx)";

	Ort::Env env( ORT_LOGGING_LEVEL_WARNING, "npr" );
	Ort::SessionOptions so;
	so.DisableMemPattern();                 // DirectML の要件
	so.SetExecutionMode( ORT_SEQUENTIAL );  // DirectML の要件
	Ort::ThrowOnError( OrtSessionOptionsAppendExecutionProvider_DML( so, 0 ) );

	Ort::Session session( env, path.c_str(), so );

	const int64_t H = 256, W = 256;
	std::vector< float > image( static_cast< size_t >( 3 * H * W ), 0.5f );
	std::vector< float > noise( static_cast< size_t >( 1 * H * W ), 0.0f );
	std::array< int64_t, 4 > image_shape{ 1, 3, H, W };
	std::array< int64_t, 4 > noise_shape{ 1, 1, H, W };

	auto mem = Ort::MemoryInfo::CreateCpu( OrtArenaAllocator, OrtMemTypeDefault );
	std::array< Ort::Value, 2 > inputs{
		Ort::Value::CreateTensor< float >( mem, image.data(), image.size(), image_shape.data(), image_shape.size() ),
		Ort::Value::CreateTensor< float >( mem, noise.data(), noise.size(), noise_shape.data(), noise_shape.size() ),
	};
	const char* input_names[]  = { "input", "noise" };
	const char* output_names[] = { "output" };

	// ウォームアップ
	auto out = session.Run( Ort::RunOptions{ nullptr }, input_names, inputs.data(), 2, output_names, 1 );

	const int n = 30;
	const auto t0 = std::chrono::high_resolution_clock::now();
	for ( int i = 0; i < n; ++i )
	{
		out = session.Run( Ort::RunOptions{ nullptr }, input_names, inputs.data(), 2, output_names, 1 );
	}
	const auto t1 = std::chrono::high_resolution_clock::now();
	const double ms = std::chrono::duration< double, std::milli >( t1 - t0 ).count() / n;

	const auto shape = out.front().GetTensorTypeAndShapeInfo().GetShape();
	ASSERT_EQ( shape.size(), 4u );
	EXPECT_EQ( shape[ 0 ], 1 );
	EXPECT_EQ( shape[ 1 ], 3 );
	EXPECT_EQ( shape[ 2 ], H );
	EXPECT_EQ( shape[ 3 ], W );

	std::cout << "[NeuralNpr] DirectML inference " << H << "x" << W << ": "
		<< ms << " ms/frame (" << ( 1000.0 / ms ) << " fps)" << std::endl;
}
