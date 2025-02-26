#pragma once
#include <memory>
#include "Application/WVP.h"
#include "Graphics/Resources/Buffers/ConstantBuffer.h"

namespace Prism
{
	namespace Gfx
	{
		class Renderer;
		class Camera;
		class Mesh;
		class Shader;
	}
	struct WVP;
	class SceneNode;

	class NodeRenderingSystem
	{
	public:
		NodeRenderingSystem(Gfx::Renderer& renderer);
		~NodeRenderingSystem();

		bool Init();

		void RenderNode(const SceneNode& node, const Gfx::Camera& camera) const;

		inline void SetVertexShader(Gfx::Shader* shader) { m_shaderVS = shader; }
		inline void SetPixelShader(Gfx::Shader* shader) { m_shaderPS = shader; }

	private:
		Gfx::Renderer& m_renderer;
		Gfx::Shader* m_shaderVS = nullptr;
		Gfx::Shader* m_shaderPS = nullptr;

		std::shared_ptr<Gfx::ConstantBuffer<WVP>> m_wvpConstantBuffer;
	};
}