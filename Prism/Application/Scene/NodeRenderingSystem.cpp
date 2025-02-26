#include "NodeRenderingSystem.h"
#include "Utils/Log.h"
#include "Application/Scene/SceneNode.h"
#include "Graphics/Renderer.h"
#include "Graphics/Utils/ResourceFactory.h"
#include "Graphics/Camera.h"
#include <Elos/Common/Assert.h>

namespace Prism
{
	NodeRenderingSystem::NodeRenderingSystem(Gfx::Renderer& renderer)
		: m_renderer(renderer)
	{
	}
	
	NodeRenderingSystem::~NodeRenderingSystem()
	{
		m_wvpConstantBuffer.reset();
	}
	
	bool NodeRenderingSystem::Init()
	{
		const auto& resourceFactory = m_renderer.GetResourceFactory();
		if (auto cbResult = resourceFactory.CreateConstantBuffer<WVP>(); !cbResult)
		{
			Log::Error("Failed to create WVP constant buffer for NodeRenderingSystem!");
			return false;
		}
		else
		{
			m_wvpConstantBuffer = std::move(cbResult.value());
			return true;
		}
	}
	
	void NodeRenderingSystem::RenderNode(const SceneNode& node, const Gfx::Camera& camera) const
	{
		// Check if we have everything we need to render
		if (!m_wvpConstantBuffer || !m_shaderVS || !m_shaderPS)
		{
			Log::Warn("NodeRenderingSystem: Missing resources for rendering");
			return;
		}

		// Check if the node has render data
		const SceneNode::RenderData& renderData = node.GetRenderData();
		if (!renderData.Mesh)
		{
			return; // Nothing to render
		}

		m_renderer.SetShader(*m_shaderVS);
		m_renderer.SetShader(*m_shaderPS);

		WVP wvp
		{
			.World      = node.GetWorldTransform().Transpose(),
			.View       = camera.GetViewMatrix().Transpose(),
			.Projection = camera.GetProjectionMatrix().Transpose()
		};

		if (auto result = m_renderer.UpdateConstantBuffer(*m_wvpConstantBuffer, wvp); !result)
		{
			Log::Error("Failed to update WVP constant buffer for node '{}'", node.GetName());

#if PRISM_BUILD_DEBUG
			Elos::ASSERT(false).Msg("Failed to update WVP constant buffer for node '{}'", node.GetName()).Throw();
#endif
			return;
		}

		DX11::IBuffer* const d3dCBuffer = m_wvpConstantBuffer->GetBuffer();
		m_renderer.SetConstantBuffers(0, Gfx::Shader::Type::Vertex, std::span(&d3dCBuffer, 1));

		m_renderer.DrawMesh(*renderData.Mesh);
	}
}