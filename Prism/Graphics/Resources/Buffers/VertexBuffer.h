#pragma once
#include "Graphics/Resources/Buffers/Buffer.h"

namespace Prism::Gfx
{
	class VertexBuffer : public Buffer
	{
	public:
		explicit VertexBuffer(bool isDynamic) : Buffer(isDynamic) {}

	public:
		u32 VertexCount = 0;
		u32 Stride = 0;
	};
}