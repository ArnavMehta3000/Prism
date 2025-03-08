// Constant buffer containing transformation matrices
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

// Vertex input structure matching your C++ data
struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

// Pixel shader input structure
struct PSInput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

#if defined(BUILD_AS_VS)

PSInput VSMain(VSInput input)
{
    PSInput output;

    // Transform the vertex position from model space to projection space
    float4 pos = float4(input.Position, 1.0f);
    pos = mul(pos, model);       // Model space to world space
    pos = mul(pos, view);        // World space to view space
    pos = mul(pos, projection);  // View space to projection space

    output.Position = pos;

    // Pass vertex color to the pixel shader
    output.Color = input.Color;

    return output;
}

#endif // BUILD_AS_VS

#if defined(BUILD_AS_PS)

float4 PSMain(PSInput input) : SV_Target
{
    // Output the interpolated color from the vertex shader
    return input.Color;
}

#endif // BUILD_AS_PS
