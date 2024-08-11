// model
Texture2D model_texture : register( t0 );		/// モデルのテクスチャ
Texture2D shadow_texture : register( t1 );		/// シャドウマップ
Texture2D paper_texture : register( t2 );		/// 紙の質感テクスチャ
Texture2D matcap_texture : register( t3 );		/// Matcap テクスチャ
Texture2D displacement_texture : register( t4 );/// ディスプレイスメントマッピング用テクスチャ
Texture2D normal_texture : register( t5 );		/// ノーマルマッピング用テクスチャ

// line
Texture2D line_texture : register( t0 );		/// 線のテクスチャ

// canvas
Texture2D pen_texture : register( t1 );			/// ペンのテクスチャ
