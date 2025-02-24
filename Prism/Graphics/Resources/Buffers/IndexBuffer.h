#pragma once
#include "Graphics/Resources/Buffers/Buffer.h"

namespace Prism::Gfx
{
	class IndexBuffer : public Buffer
	{
	public:
		explicit IndexBuffer(bool isDynamic) : Buffer(isDynamic) {}

	public:
		u32 IndexCount = 0;
	};
}