xof 0303txt 0032


template VertexDuplicationIndices { 
 <b8d65549-d7c9-4995-89cf-53a9a8b031e3>
 DWORD nIndices;
 DWORD nOriginalVertices;
 array DWORD indices[nIndices];
}
template XSkinMeshHeader {
 <3cf169ce-ff7c-44ab-93c0-f78f62d172e2>
 WORD nMaxSkinWeightsPerVertex;
 WORD nMaxSkinWeightsPerFace;
 WORD nBones;
}
template SkinWeights {
 <6f0d123b-bad2-4167-a0d0-80224f25fabb>
 STRING transformNodeName;
 DWORD nWeights;
 array DWORD vertexIndices[nWeights];
 array float weights[nWeights];
 Matrix4x4 matrixOffset;
}

Frame RootFrame {

  FrameTransformMatrix {
    1.000000,0.000000,0.000000,0.000000,
    0.000000,-0.000000,1.000000,0.000000,
    0.000000,1.000000,0.000000,0.000000,
    0.000000,0.000000,0.000000,1.000000;;
  }
Frame building-100-0 {

  FrameTransformMatrix {
    1.000000,0.000000,0.000000,0.000000,
    0.000000,1.000000,0.000000,0.000000,
    0.000000,0.000000,1.000000,0.000000,
    0.000000,0.000000,0.000000,1.000000;;
  }
Mesh {
52;
1.990000; 1.990000; 0.000000;,
1.990000; 1.990000; 10.000000;,
1.990000; -0.000000; 10.000000;,
1.990000; 0.000000; 0.000000;,
1.990000; 0.000000; 0.000000;,
1.990000; -0.000000; 10.000000;,
-0.000000; 0.000000; 10.000000;,
-0.000000; 0.000000; 0.000000;,
-0.000000; 0.000000; 0.000000;,
-0.000000; 0.000000; 10.000000;,
0.000000; 1.990000; 10.000000;,
0.000000; 1.990000; 0.000000;,
1.990000; 1.990000; 10.000000;,
1.990000; 1.990000; 0.000000;,
0.000000; 1.990000; 0.000000;,
0.000000; 1.990000; 10.000000;,
1.990000; 1.990000; 10.000000;,
0.000000; 1.990000; 10.000000;,
0.100000; 1.900000; 10.000000;,
1.900000; 1.900000; 10.000000;,
0.000000; 1.990000; 10.000000;,
-0.000000; 0.000000; 10.000000;,
0.100000; 0.100000; 10.000000;,
0.100000; 1.900000; 10.000000;,
-0.000000; 0.000000; 10.000000;,
1.990000; -0.000000; 10.000000;,
1.900000; 0.100000; 10.000000;,
0.100000; 0.100000; 10.000000;,
1.990000; -0.000000; 10.000000;,
1.990000; 1.990000; 10.000000;,
1.900000; 1.900000; 10.000000;,
1.900000; 0.100000; 10.000000;,
1.900000; 0.100000; 10.000000;,
1.900000; 1.900000; 10.000000;,
1.900000; 1.900000; 9.900000;,
1.900000; 0.100000; 9.900000;,
0.100000; 0.100000; 10.000000;,
1.900000; 0.100000; 10.000000;,
1.900000; 0.100000; 9.900000;,
0.100000; 0.100000; 9.900000;,
0.100000; 1.900000; 10.000000;,
0.100000; 0.100000; 10.000000;,
0.100000; 0.100000; 9.900000;,
0.100000; 1.900000; 9.900000;,
1.900000; 1.900000; 10.000000;,
0.100000; 1.900000; 10.000000;,
0.100000; 1.900000; 9.900000;,
1.900000; 1.900000; 9.900000;,
1.900000; 1.900000; 9.900000;,
0.100000; 1.900000; 9.900000;,
0.100000; 0.100000; 9.900000;,
1.900000; 0.100000; 9.900000;;
13;
4; 0, 3, 2, 1;,
4; 4, 7, 6, 5;,
4; 8, 11, 10, 9;,
4; 12, 15, 14, 13;,
4; 16, 19, 18, 17;,
4; 20, 23, 22, 21;,
4; 24, 27, 26, 25;,
4; 28, 31, 30, 29;,
4; 32, 35, 34, 33;,
4; 36, 39, 38, 37;,
4; 40, 43, 42, 41;,
4; 44, 47, 46, 45;,
4; 48, 51, 50, 49;;
  MeshMaterialList {
    1;
    13;
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0;;
  Material Mat1 {
    1.0; 1.0; 1.0; 1.0;;
    1.0;
    1.0; 1.0; 1.0;;
    0.0; 0.0; 0.0;;
  TextureFilename {    "building-100-0.png";  }
  }  // End of Material
    }  //End of MeshMaterialList
  MeshNormals {
52;
    0.707083; 0.707083; 0.000000;,
    0.408246; 0.408246; 0.816492;,
    0.408246; -0.408246; 0.816492;,
    0.707083; -0.707083; 0.000000;,
    0.707083; -0.707083; 0.000000;,
    0.408246; -0.408246; 0.816492;,
    -0.408246; -0.408246; 0.816492;,
    -0.707083; -0.707083; 0.000000;,
    -0.707083; -0.707083; 0.000000;,
    -0.408246; -0.408246; 0.816492;,
    -0.408246; 0.408246; 0.816492;,
    -0.707083; 0.707083; 0.000000;,
    0.408246; 0.408246; 0.816492;,
    0.707083; 0.707083; 0.000000;,
    -0.707083; 0.707083; 0.000000;,
    -0.408246; 0.408246; 0.816492;,
    0.408246; 0.408246; 0.816492;,
    -0.408246; 0.408246; 0.816492;,
    0.408246; -0.408246; 0.816492;,
    -0.408246; -0.408246; 0.816492;,
    -0.408246; 0.408246; 0.816492;,
    -0.408246; -0.408246; 0.816492;,
    0.408246; 0.408246; 0.816492;,
    0.408246; -0.408246; 0.816492;,
    -0.408246; -0.408246; 0.816492;,
    0.408246; -0.408246; 0.816492;,
    -0.408246; 0.408246; 0.816492;,
    0.408246; 0.408246; 0.816492;,
    0.408246; -0.408246; 0.816492;,
    0.408246; 0.408246; 0.816492;,
    -0.408246; -0.408246; 0.816492;,
    -0.408246; 0.408246; 0.816492;,
    -0.408246; 0.408246; 0.816492;,
    -0.408246; -0.408246; 0.816492;,
    -0.577349; -0.577349; 0.577349;,
    -0.577349; 0.577349; 0.577349;,
    0.408246; 0.408246; 0.816492;,
    -0.408246; 0.408246; 0.816492;,
    -0.577349; 0.577349; 0.577349;,
    0.577349; 0.577349; 0.577349;,
    0.408246; -0.408246; 0.816492;,
    0.408246; 0.408246; 0.816492;,
    0.577349; 0.577349; 0.577349;,
    0.577349; -0.577349; 0.577349;,
    -0.408246; -0.408246; 0.816492;,
    0.408246; -0.408246; 0.816492;,
    0.577349; -0.577349; 0.577349;,
    -0.577349; -0.577349; 0.577349;,
    -0.577349; -0.577349; 0.577349;,
    0.577349; -0.577349; 0.577349;,
    0.577349; 0.577349; 0.577349;,
    -0.577349; 0.577349; 0.577349;;
13;
4; 0, 3, 2, 1;,
4; 4, 7, 6, 5;,
4; 8, 11, 10, 9;,
4; 12, 15, 14, 13;,
4; 16, 19, 18, 17;,
4; 20, 23, 22, 21;,
4; 24, 27, 26, 25;,
4; 28, 31, 30, 29;,
4; 32, 35, 34, 33;,
4; 36, 39, 38, 37;,
4; 40, 43, 42, 41;,
4; 44, 47, 46, 45;,
4; 48, 51, 50, 49;;
}  //End of MeshNormals
MeshTextureCoords {
52;
0.874023;0.999023;,
0.874023;0.000977;,
0.749023;0.000977;,
0.749023;0.999023;,
0.749023;0.999023;,
0.749023;0.000977;,
0.624023;0.000977;,
0.624023;0.999023;,
0.624023;0.999023;,
0.624023;0.000977;,
0.499023;0.000977;,
0.499023;0.999023;,
0.874023;0.000977;,
0.874023;0.999023;,
0.999023;0.999023;,
0.999023;0.000977;,
0.496094;0.503906;,
0.001953;0.503906;,
0.018555;0.517578;,
0.482422;0.517578;,
0.001953;0.503906;,
0.001953;0.998047;,
0.018555;0.981445;,
0.018555;0.517578;,
0.001953;0.998047;,
0.496094;0.998047;,
0.482422;0.981445;,
0.018555;0.981445;,
0.496094;0.998047;,
0.496094;0.503906;,
0.482422;0.517578;,
0.482422;0.981445;,
0.482422;0.981445;,
0.482422;0.517578;,
0.458984;0.541016;,
0.458984;0.958008;,
0.018555;0.981445;,
0.482422;0.981445;,
0.458984;0.958008;,
0.041992;0.958008;,
0.018555;0.517578;,
0.018555;0.981445;,
0.041992;0.958008;,
0.041992;0.541016;,
0.482422;0.517578;,
0.018555;0.517578;,
0.041992;0.541016;,
0.458984;0.541016;,
0.458984;0.541016;,
0.041992;0.541016;,
0.041992;0.958008;,
0.458984;0.958008;;
}  //End of MeshTextureCoords
MeshVertexColors {
52;
0;1.000000;1.000000;1.000000;1.000000;,
1;1.000000;1.000000;1.000000;1.000000;,
2;1.000000;1.000000;1.000000;1.000000;,
3;1.000000;1.000000;1.000000;1.000000;,
4;1.000000;1.000000;1.000000;1.000000;,
5;1.000000;1.000000;1.000000;1.000000;,
6;1.000000;1.000000;1.000000;1.000000;,
7;1.000000;1.000000;1.000000;1.000000;,
8;1.000000;1.000000;1.000000;1.000000;,
9;1.000000;1.000000;1.000000;1.000000;,
10;1.000000;1.000000;1.000000;1.000000;,
11;1.000000;1.000000;1.000000;1.000000;,
12;1.000000;1.000000;1.000000;1.000000;,
13;1.000000;1.000000;1.000000;1.000000;,
14;1.000000;1.000000;1.000000;1.000000;,
15;1.000000;1.000000;1.000000;1.000000;,
16;1.000000;1.000000;1.000000;1.000000;,
17;1.000000;1.000000;1.000000;1.000000;,
18;1.000000;1.000000;1.000000;1.000000;,
19;1.000000;1.000000;1.000000;1.000000;,
20;1.000000;1.000000;1.000000;1.000000;,
21;1.000000;1.000000;1.000000;1.000000;,
22;1.000000;1.000000;1.000000;1.000000;,
23;1.000000;1.000000;1.000000;1.000000;,
24;1.000000;1.000000;1.000000;1.000000;,
25;1.000000;1.000000;1.000000;1.000000;,
26;1.000000;1.000000;1.000000;1.000000;,
27;1.000000;1.000000;1.000000;1.000000;,
28;1.000000;1.000000;1.000000;1.000000;,
29;1.000000;1.000000;1.000000;1.000000;,
30;1.000000;1.000000;1.000000;1.000000;,
31;1.000000;1.000000;1.000000;1.000000;,
32;1.000000;1.000000;1.000000;1.000000;,
33;1.000000;1.000000;1.000000;1.000000;,
34;1.000000;1.000000;1.000000;1.000000;,
35;1.000000;1.000000;1.000000;1.000000;,
36;1.000000;1.000000;1.000000;1.000000;,
37;1.000000;1.000000;1.000000;1.000000;,
38;1.000000;1.000000;1.000000;1.000000;,
39;1.000000;1.000000;1.000000;1.000000;,
40;1.000000;1.000000;1.000000;1.000000;,
41;1.000000;1.000000;1.000000;1.000000;,
42;1.000000;1.000000;1.000000;1.000000;,
43;1.000000;1.000000;1.000000;1.000000;,
44;1.000000;1.000000;1.000000;1.000000;,
45;1.000000;1.000000;1.000000;1.000000;,
46;1.000000;1.000000;1.000000;1.000000;,
47;1.000000;1.000000;1.000000;1.000000;,
48;1.000000;1.000000;1.000000;1.000000;,
49;1.000000;1.000000;1.000000;1.000000;,
50;1.000000;1.000000;1.000000;1.000000;,
51;1.000000;1.000000;1.000000;1.000000;;
}  //End of MeshVertexColors
 }
}
}
