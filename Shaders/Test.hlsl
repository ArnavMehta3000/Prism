// Common structures shared between VS and PS
struct VSInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

// Vertex Shader Section
#if defined(BUILD_AS_VS)

PSInput VSMain(VSInput input)
{
    PSInput output;

    // Simple pass-through transformation
    output.Position = float4(input.Position, 1.0f);
    output.TexCoord = input.TexCoord;

    return output;
}

#endif // BUILD_AS_VS

// Pixel Shader Section
#if defined(BUILD_AS_PS)

float4 PSMain(PSInput input) : SV_Target
{
    // Simple white output
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

#endif // BUILD_AS_PS
