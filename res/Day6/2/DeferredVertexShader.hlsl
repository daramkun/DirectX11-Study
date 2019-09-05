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
    float4 position3 : POSITION1;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

cbuffer TRANSFORM
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};

VERTEX_OUT main (VERTEX_IN vin)
{
    VERTEX_OUT vout;
    vout.position1 = vout.position2 = mul(float4(vin.position, 1), world);
    vout.position1 = mul(vout.position1, view);
    vout.position1 = mul(vout.position1, proj);
    vout.position3 = vout.position1;
    vout.normal = mul(vin.normal, (float3x3) world);
    vout.texcoord = vin.texcoord;
    vout.color = vin.color;

    return vout;
}