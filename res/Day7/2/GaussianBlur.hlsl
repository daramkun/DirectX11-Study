Texture2D<float4> inputTexture : register(t0);
RWTexture2D<float4> outputTexture : register(u0);

#define GAUSSIAN_BLUR_FACTOR								(1 / 256.0f)

static float gaussianBlurFilterMask[] =
{
    1, 4, 6, 4, 1,
	4, 16, 24, 16, 4,
	6, 24, 36, 24, 6,
	4, 16, 24, 16, 4,
	1, 4, 6, 4, 1
};

inline float3 LoadPixel(int3 xyz, int2 size)
{
    return inputTexture.Load(clamp(xyz, int3(0, 0, 0), int3(size, 0))).rgb;
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    int2 textureSize;
    inputTexture.GetDimensions(textureSize.x, textureSize.y);

    int3 intId = int3(dispatchThreadId);

    float3 result = float3(0, 0, 0);
    for (int my = 0; my < 5; ++my)
    {
        for (int mx = 0; mx < 5; ++mx)
        {
            int3 offset = int3(mx - 5 / 2, my - 5 / 2, 0);
            float3 loaded = LoadPixel(intId + offset, textureSize);
            float filterValue = gaussianBlurFilterMask[my * 5 + mx] * GAUSSIAN_BLUR_FACTOR;
            result += (loaded * filterValue);
        }
    }

    outputTexture[dispatchThreadId.xy] = float4(result, 1);
}