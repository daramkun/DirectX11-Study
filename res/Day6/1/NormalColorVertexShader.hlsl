struct VERTEX_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

struct VERTEX_OUT
{
    float4 position1 : SV_Position;
    float4 position2 : POSITION;
    float4 color : COLOR;
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
    vout.position1 = float4(vin.position, 1);
    vout.position1 = mul(vout.position1, worldTransform);
    vout.position1 = mul(vout.position1, viewTransform);
    vout.position1 = mul(vout.position1, projTransform);
    vout.position2 = vout.position1;
    vout.color = float4(vin.normal, 1);

    return vout;
}