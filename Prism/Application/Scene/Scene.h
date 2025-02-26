#pragma once
#include "Application/Scene/SceneNode.h"
#include <unordered_map>

namespace Prism
{
	namespace Gfx
	{
		class Camera;
		class Renderer;
		class Shader;
		class Mesh;
	}

	class NodeRenderingSystem;

	class Scene
	{
	public:
		Scene(const Elos::String& name);
		~Scene();

		void Update(f32 deltaTime);
		void Render(Gfx::Renderer& renderer, Gfx::Camera& camera);
		bool Init(Gfx::Renderer& renderer);

		NODISCARD inline SceneNode* GetRootNode() const { return m_rootNode.get(); }
		NODISCARD inline const std::string& GetName() const { return m_name; }
		NODISCARD SceneNode* FindNode(const std::string& name) const;
		NODISCARD SceneNode* CreateNode(const std::string& name, SceneNode* parent = nullptr);
		NODISCARD std::shared_ptr<Gfx::Mesh> GetMesh(const std::string& name) const;

		void AddMesh(const std::string& name, std::shared_ptr<Gfx::Mesh> mesh);
		void SetVertexShader(Gfx::Shader* shader);
		void SetPixelShader(Gfx::Shader* shader);

	private:
		Elos::String                                                m_name;
		std::unique_ptr<SceneNode>                                  m_rootNode;
		std::unordered_map<std::string, std::shared_ptr<Gfx::Mesh>> m_meshes;
		std::unordered_map<std::string, SceneNode*>                 m_nodeRegistry;
		std::unique_ptr<NodeRenderingSystem>                        m_renderingSystem;
	};
}