#include "Mesh.h"
#include <Elos/Common/Assert.h>

namespace Prism::Gfx
{
	Mesh::~Mesh() noexcept
	{
		m_vertexBuffer.reset();
		m_indexBuffer.reset();
	}

	void Mesh::Bind(DX11::IDeviceContext* context) const noexcept
	{
#if PRISM_BUILD_DEBUG
		Elos::ASSERT_NOT_NULL(context);
		Elos::ASSERT_NOT_NULL(GetVertexBuffer());
		Elos::ASSERT_NOT_NULL(GetIndexBuffer());
#endif

		if (!context || !m_vertexBuffer || !m_indexBuffer)
		{
			return;
		}

		const UINT stride = m_vertexBuffer->Stride;
		const UINT offset = 0;
		
		DX11::IBuffer* const vb = m_vertexBuffer->GetBuffer();
		DX11::IBuffer* const ib = m_indexBuffer->GetBuffer();

		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(m_topology);
	}
}