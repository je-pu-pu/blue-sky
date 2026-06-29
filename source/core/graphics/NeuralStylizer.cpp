#include "NeuralStylizer.h"

#include <onnxruntime/dml_provider_factory.h>

#include <common/exception.h>

#include <array>
#include <filesystem>

namespace core::graphics
{

namespace
{

// DirectML EP を有効にした SessionOptions を構築して返す。
Ort::SessionOptions make_session_options()
{
	Ort::SessionOptions options;
	options.DisableMemPattern();                // DirectML の要件
	options.SetExecutionMode( ORT_SEQUENTIAL ); // DirectML の要件
	Ort::ThrowOnError( OrtSessionOptionsAppendExecutionProvider_DML( options, 0 ) );
	return options;
}

std::wstring to_wide( const char* s )
{
	return std::filesystem::path( s ).wstring();
}

} // namespace

NeuralStylizer::NeuralStylizer( const char* model_path )
	: env_( ORT_LOGGING_LEVEL_WARNING, "neural_stylizer" )
	, session_options_( make_session_options() )
	, session_( env_, to_wide( model_path ).c_str(), session_options_ )
	, memory_info_( Ort::MemoryInfo::CreateCpu( OrtArenaAllocator, OrtMemTypeDefault ) )
{
}

void NeuralStylizer::stylize( int width, int height,
	const float* image_nchw, const float* noise, float* out_nchw )
{
	const int64_t h = height;
	const int64_t w = width;
	const std::array< int64_t, 4 > image_shape{ 1, 3, h, w };
	const std::array< int64_t, 4 > noise_shape{ 1, 1, h, w };
	const size_t image_size = static_cast< size_t >( 3 * h * w );
	const size_t noise_size = static_cast< size_t >( 1 * h * w );

	std::array< Ort::Value, 2 > inputs{
		Ort::Value::CreateTensor< float >( memory_info_,
			const_cast< float* >( image_nchw ), image_size, image_shape.data(), image_shape.size() ),
		Ort::Value::CreateTensor< float >( memory_info_,
			const_cast< float* >( noise ), noise_size, noise_shape.data(), noise_shape.size() ),
	};

	const char* input_names[]  = { "input", "noise" };
	const char* output_names[] = { "output" };

	auto outputs = session_.Run( Ort::RunOptions{ nullptr },
		input_names, inputs.data(), inputs.size(), output_names, 1 );

	const auto shape = outputs.front().GetTensorTypeAndShapeInfo().GetShape();
	if ( shape.size() != 4 || shape[ 1 ] != 3 || shape[ 2 ] != h || shape[ 3 ] != w )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "NeuralStylizer: unexpected output shape." );
	}

	const float* out = outputs.front().GetTensorData< float >();
	std::copy( out, out + image_size, out_nchw );
}

} // namespace core::graphics
