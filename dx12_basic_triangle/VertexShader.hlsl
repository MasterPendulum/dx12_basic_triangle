
// VertexShader.hlsl
// 頂点シェーダ


// 三角形の頂点データ
struct Vertex
{
    float3 position : POSITION;
    float3 color : COLOR;
};

// 定数バッファ
cbuffer ConstantBuffer : register(b0)
{
    float4x4 objToProj;
};

// ピクセルシェーダへの出力
struct V2P
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

V2P main(Vertex input)
{
    V2P output;
    output.position = mul(float4(input.position, 1.0f), objToProj);
    output.color = float4(input.color, 1.0f);
    return output;
}
