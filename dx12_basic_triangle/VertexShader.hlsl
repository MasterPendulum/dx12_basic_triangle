
// VertexShader.hlsl
// ���_�V�F�[�_


// �O�p�`�̒��_�f�[�^
struct Vertex
{
    float3 position : POSITION;
    float3 color : COLOR;
};

// �萔�o�b�t�@
cbuffer ConstantBuffer : register(b0)
{
    float4x4 objToProj;
};

// �s�N�Z���V�F�[�_�ւ̏o��
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
