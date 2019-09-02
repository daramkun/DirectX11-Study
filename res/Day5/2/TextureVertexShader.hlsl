struct VERTEX_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

struct VERTEX_OUT
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

cbuffer TRANSFORM
{
    float4x4 worldTransform;
    float4x4 viewTransform;
    float4x4 projTransform;
};

VERTEX_OUT main(VERTEX_IN vin)
{
    VERTEX_OUT vout;
    vout.position = float4(vin.position, 1);
    vout.position = mul(vout.position, worldTransform);
    vout.position = mul(vout.position, viewTransform);
    vout.position = mul(vout.position, projTransform);
    vout.texcoord = vin.texcoord;

    return vout;
}