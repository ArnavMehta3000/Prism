#pragma once
#include "Graphics/Resources/Buffers/Buffer.h"

namespace Prism::Gfx
{
	template <typename T>
	concept ConstantBufferType = std::is_trivially_copyable_v<T> && (sizeof(T) % 16 == 0);

	template <ConstantBufferType T>
	class ConstantBuffer : public Buffer
	{
	public:
		ConstantBuffer() : Buffer(true) {}

		std::expected<void, BufferError> Update(DX11::IDeviceContext* context, const T& data)
		{
			return Buffer::Update(context, &data, sizeof(T));
		}
	};
}