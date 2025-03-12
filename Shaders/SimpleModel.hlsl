/*
* This file contains the shader to render a loaded GLTF model for the Prism renderer.
* The pixel shader (PSMain) fakes directional lighting since we don't consider lighting
*/


cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix ModelMat;
    matrix ViewMat;
    matrix ProjectionMat;
};

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float3 Tangent  : TANGENT;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD0;
};

#if defined(BUILD_AS_VS)

PSInput VSMain(VSInput input)
{
    PSInput output;

    // Transform the vertex position from model space to projection space
    float4 pos = float4(input.Position, 1.0f);
    pos = mul(pos, ModelMat);
    pos = mul(pos, ViewMat);
    pos = mul(pos, ProjectionMat);
    
    output.Position = pos;

    // Transform normal and tangets to world space
    output.Normal = normalize(mul(input.Normal, (float3x3)ModelMat));
    
    output.Color = float4(input.Normal, 1.0f);
    output.TexCoord = input.TexCoord;

    return output;
}

#endif // BUILD_AS_VS

#if defined(BUILD_AS_PS)

float4 PSMain(PSInput input) : SV_Target
{
    // Simple directional light calculation
    float3 lightDir = normalize(float3(0.577f, -0.577f, 0.577f));
    float3 normal = normalize(input.Normal);

    // Calculate diffuse lighting
    float diffuseAmount = max(dot(normal, -lightDir), 0.0f);
    float3 diffuse = diffuseAmount * float3(1.0f, 1.0f, 1.0f);

    // Add ambient light
    float3 ambient = float3(0.3f, 0.3f, 0.3f);
    float3 lighting = ambient + diffuse;

    // Get base color from vertex color
    float4 baseColor = input.Color;

    // Apply lighting to base color
    float4 finalColor = float4(baseColor.rgb * lighting, baseColor.a);

    return finalColor;
}

#endif // BUILD_AS_PS
