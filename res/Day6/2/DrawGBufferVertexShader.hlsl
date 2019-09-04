struct VERTEX_OUT
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

cbuffer TRANSFORM
{
    float4x4 world;
};

VERTEX_OUT main (uint vertexId : SV_VertexID)
{
    VERTEX_OUT vout;

    switch (vertexId)
    {
        case 0:
            vout.position = float4(-0.5f, +0.5f, 0, 1);
            vout.texcoord = float2(0, 0);
            break;

        case 1:
            vout.position = float4(+0.5f, +0.5f, 0, 1);
            vout.texcoord = float2(1, 0);
            break;

        case 2:
            vout.position = float4(-0.5f, -0.5f, 0, 1);
            vout.texcoord = float2(0, 1);
            break;

        case 3:
            vout.position = float4(+0.5f, -0.5f, 0, 1);
            vout.texcoord = float2(1, 1);
            break;
    }
    vout.position = mul(vout.position, world);

    return vout;
}