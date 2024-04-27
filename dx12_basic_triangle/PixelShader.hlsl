
// PixelShader.hlsl
// ピクセルシェーダ


// 頂点シェーダから渡ってくるもの
struct V2P
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(V2P input) : SV_Target
{
    // 補間された頂点カラーをそのまま出すだけ
    return input.color;
}
