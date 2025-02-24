#pragma once
#include "Graphics/DX11Types.h"
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Common/String.h>
#include <expected>

namespace Prism::Gfx
{
	class Buffer
	{
		friend class ResourceFactory;
	public:
		struct BufferError
		{
			enum class Type
			{
				CreateBufferFailed,
				InvalidBufferSize,
				MapFailed,
				UpdateFailed
			};

			Type Type;
			HRESULT ErrorCode;
			Elos::String Message;
		};

	public:
		explicit Buffer(const bool isDynamic = false) : m_isDynamic(isDynamic) {}
		virtual ~Buffer() { m_buffer.Reset(); }

		inline NODISCARD DX11::IBuffer* GetBuffer() const { return m_buffer.Get(); }
		inline NODISCARD bool IsDynamic() const { return m_isDynamic; }

		std::expected<void, BufferError> Update(DX11::IDeviceContext* context, const void* data, const size_t size)
		{
			if (!m_isDynamic)
			{
				return std::unexpected(BufferError{ BufferError::Type::UpdateFailed, E_FAIL, "Buffer is not dynamic" });
			}

			D3D11_MAPPED_SUBRESOURCE mapped;
			HRESULT hr = context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			if (FAILED(hr))
			{
				return std::unexpected(BufferError{ BufferError::Type::MapFailed, hr, "Failed to map buffer" });
			}

			std::memcpy(mapped.pData, data, size);
			context->Unmap(m_buffer.Get(), 0);

			return {};
		}

	protected:
		HRESULT InitInternal(DX11::IDevice* device, const D3D11_BUFFER_DESC* bufferDesc, const D3D11_SUBRESOURCE_DATA* subresourceData)
		{
			return device->CreateBuffer(bufferDesc, subresourceData, m_buffer.GetAddressOf());
		}

	protected:
		bool m_isDynamic;
		ComPtr<DX11::IBuffer> m_buffer;
	};
}