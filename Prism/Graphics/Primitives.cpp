#include "Primitives.h"

namespace Prism::Gfx::Primitives
{
    const std::array<Cube::VertexTypes::Position, 8> Cube::VertexPosition =
    {
        DirectX::VertexPosition(DirectX::XMFLOAT3(-1, -1, -1)),
        DirectX::VertexPosition(DirectX::XMFLOAT3(1, -1, -1)),
        DirectX::VertexPosition(DirectX::XMFLOAT3(1, 1, -1)),
        DirectX::VertexPosition(DirectX::XMFLOAT3(-1, 1, -1)),
        DirectX::VertexPosition(DirectX::XMFLOAT3(-1, -1, 1)),
        DirectX::VertexPosition(DirectX::XMFLOAT3(1, -1, 1)),
        DirectX::VertexPosition(DirectX::XMFLOAT3(1, 1, 1)),
        DirectX::VertexPosition(DirectX::XMFLOAT3(-1, 1, 1))
    };
    
    const std::array<Cube::VertexTypes::PositionColor, 8> Cube::VertexPositionColor =
    {
        DirectX::VertexPositionColor(DirectX::XMFLOAT3(-1, -1, -1), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)),
        DirectX::VertexPositionColor(DirectX::XMFLOAT3(1, -1, -1), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)),
        DirectX::VertexPositionColor(DirectX::XMFLOAT3(1, 1, -1), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)),
        DirectX::VertexPositionColor(DirectX::XMFLOAT3(-1, 1, -1), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)),
        DirectX::VertexPositionColor(DirectX::XMFLOAT3(-1, -1, 1), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)),
        DirectX::VertexPositionColor(DirectX::XMFLOAT3(1, -1, 1), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)),
        DirectX::VertexPositionColor(DirectX::XMFLOAT3(1, 1, 1), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)),
        DirectX::VertexPositionColor(DirectX::XMFLOAT3(-1, 1, 1), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)),
    };

    const std::array<u32, 36> Cube::Indices
    {
        0, 1, 3, 3, 1, 2,
        1, 5, 2, 2, 5, 6,
        5, 4, 6, 6, 4, 7,
        4, 0, 7, 7, 0, 3,
        3, 2, 7, 7, 2, 6,
        4, 5, 0, 0, 5, 1
    };
}