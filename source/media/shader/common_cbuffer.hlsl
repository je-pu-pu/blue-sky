#include "common_const.hlsl"

cbuffer GameConstantBuffer : register( b0 )
{
	float ScreenWidth;
	float ScreenHeight;
};

cbuffer FrameConstantBuffer : register( b1 )
{
	/// @todo ViewProjection としてまとめる？
    row_major matrix View;			// ビュー変換行列
    row_major matrix Projection;	// プロジェクション変換行列
	float4 Light;					// 光源の向き ( 正規化済み ) 
	float Time;						// シーン開始からの経過秒数
	uint TimeBeat;					// 現在の音楽の BPM ?
	float TessFactor;				// テッセレーションの分割数
	float BeatProgress;				// ビート進行度 (1.0 -> 0.0)
};

cbuffer ObjectConstantBuffer : register( b2 )
{
    row_major matrix World;			// ワールド変換行列
	float4 ObjectColor;				// オブジェクトの色
};

cbuffer FrameDrawingConstantBuffer : register( b4 )
{
	float4 ShadowColor;
	float4 ShadowPaperColor;
	float DrawingAccent;
	uint LineType;
}

cbuffer BoneConstantBuffer : register( b5 )
{
    row_major matrix BoneMatrix[ MaxBones ];
};

cbuffer ShadowMapConstantBuffer : register( b10 )
{
    row_major matrix ShadowViewProjection[ ShadowMapCascadeLevels ];
	float4 ShadowMapViewDepthPerCascadeLevel;
};
