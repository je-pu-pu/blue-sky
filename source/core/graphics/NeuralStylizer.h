#pragma once

#include <onnxruntime/onnxruntime_cxx_api.h>

#include <string>
#include <vector>

namespace core::graphics
{

/**
 * 確率的スタイル化ネット(ONNX)を ONNX Runtime + DirectML で推論する。
 *
 * モデル(starry_boil_small.onnx 等)の入力は NCHW float[0,1]:
 *   "input" = [1,3,H,W]（RGB プレーナ）, "noise" = [1,1,H,W]（フレーム毎ノイズ）
 * 出力 "output" = [1,3,H,W]。H/W は可変(dynamic axes)。
 *
 * フレーム毎にノイズを変えると筆致が描き直され、リアルタイムで「ボイリング」が出る。
 */
class NeuralStylizer
{
public:
	/// model_path の ONNX を DirectML EP でロードする（失敗時は例外）。
	explicit NeuralStylizer( const char* model_path );

	/**
	 * 1 フレームをスタイル化する。バッファは呼び出し側が確保する。
	 * @param width,height  画像サイズ
	 * @param image_nchw    入力 RGB プレーナ float[0,1]、サイズ 3*width*height
	 * @param noise         入力ノイズ float、サイズ 1*width*height
	 * @param out_nchw      出力 RGB プレーナ float[0,1]、サイズ 3*width*height
	 */
	void stylize( int width, int height,
		const float* image_nchw, const float* noise, float* out_nchw );

private:
	Ort::Env env_;
	Ort::SessionOptions session_options_;
	Ort::Session session_;
	Ort::MemoryInfo memory_info_;

}; // class NeuralStylizer

} // namespace core::graphics
