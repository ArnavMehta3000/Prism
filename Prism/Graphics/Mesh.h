#pragma once
#include "StandardTypes.h"
#include "Graphics/DX11Types.h"
#include "Graphics/Resources/Buffers/VertexBuffer.h"
#include "Graphics/Resources/Buffers/IndexBuffer.h"
#include <Elos/Common/String.h>
#include <Elos/Common/FunctionMacros.h>
#include <memory>

namespace Prism::Gfx
{
	class Renderer;

	class Mesh
	{
		friend class ResourceFactory;
	public:
		struct MeshError
		{
			enum class Type
			{
				CreateVertexBufferFailed,
				CreateIndexBufferFailed,
			};

			Type Type;
			HRESULT ErrorCode;
			Elos::String Message;
		};

		struct MeshDesc
		{
			D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			u32 VertexStride                  = 0;
			bool DynamicVB                    = false;
			bool DynamicIB                    = false;
		};

	public:
		~Mesh() noexcept;
		void Bind(DX11::IDeviceContext* context) const noexcept;

		NODISCARD inline D3D11_PRIMITIVE_TOPOLOGY GetTopology() const noexcept { return m_topology; }
		NODISCARD inline VertexBuffer* GetVertexBuffer() const noexcept { return m_vertexBuffer.get(); }
		NODISCARD inline IndexBuffer* GetIndexBuffer() const noexcept { return m_indexBuffer.get(); }

	private:
		Mesh() noexcept = default;

	private:
		std::shared_ptr<VertexBuffer> m_vertexBuffer;
		std::shared_ptr<IndexBuffer>  m_indexBuffer;
		D3D11_PRIMITIVE_TOPOLOGY      m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	};
}