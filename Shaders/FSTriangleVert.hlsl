/*
* Only the vertex shader half of FSTriangle.hlsl
*/


struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

#if defined(BUILD_AS_VS)


PSInput VSMain(uint vertexID : SV_VertexID)
{
    PSInput output;

    // This creates a large triangle that covers the entire screen
    float2 position = float2(
        (vertexID == 2) ? 3.0 : -1.0,  // x coordinate
        (vertexID == 0) ? 3.0 : -1.0   // y coordinate
    );

    output.Position = float4(position, 0.0, 1.0);
    output.TexCoord = (position + 1.0) / 2.0;

    return output;
}

#endif // BUILD_AS_VS
