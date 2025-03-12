#include "Mesh.h"
#include <Graphics/Renderer.h>
#include <Elos/Common/Assert.h>

namespace Prism::Gfx
{
	Mesh::~Mesh() noexcept
	{
		m_vertexBuffer.reset();
		m_indexBuffer.reset();
	}
	
	void Mesh::Render(const Renderer& renderer) const noexcept
	{
#if PRISM_BUILD_DEBUG
		Elos::ASSERT_NOT_NULL(GetVertexBuffer());
		Elos::ASSERT_NOT_NULL(GetIndexBuffer());
#endif

		if (!m_vertexBuffer || !m_indexBuffer)
		{
			return;
		}

		const u32 offset = 0;
		const VertexBuffer* vb[] = { m_vertexBuffer.get() };

		renderer.SetIndexBuffer(*m_indexBuffer);
		renderer.SetVertexBuffers(0, std::span{vb}, std::span(&offset, 1));
		renderer.SetPrimitiveTopology(m_topology);

		renderer.DrawIndexed(m_indexBuffer->IndexCount, 0, 0);
	}
}