
// PixelShader.hlsl
// �s�N�Z���V�F�[�_


// ���_�V�F�[�_����n���Ă������
struct V2P
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(V2P input) : SV_Target
{
    // ��Ԃ��ꂽ���_�J���[�����̂܂܏o������
    return input.color;
}
