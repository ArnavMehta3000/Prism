#pragma once
#include "StandardTypes.h"
#include <VertexTypes.h>
#include <array>

namespace Prism::Gfx::Primitives
{
	class Cube
	{
	public:
		struct VertexTypes
		{
			using Position = DirectX::VertexPosition;
			using PositionColor = DirectX::VertexPositionColor;
		};

		static const std::array<VertexTypes::Position, 8> VertexPosition;
		static const std::array<VertexTypes::PositionColor, 8> VertexPositionColor;
		static const std::array<u32, 36> Indices;
	};
}
