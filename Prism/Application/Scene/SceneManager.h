#pragma once
#include "StandardTypes.h"
#include "Application/Scene/Scene.h"
#include <Elos/Common/String.h>
#include <memory>
#include <unordered_map>

namespace Prism
{
	class Scene;
	
	namespace Gfx
	{
		class Renderer;
		class Camera;
		class ResourceFactory;
	}

	class SceneManager
	{
	public:
		SceneManager(const Gfx::ResourceFactory& resourceFactory);
		~SceneManager();

		Scene* CreateScene(const std::string& name);
		Scene* LoadScene(const std::string& name); // This would load from a file in a full implementation
		void DestroyScene(const std::string& name);
		Scene* GetScene(const std::string& name) const;

		void SetActiveScene(const std::string& name);
		Scene* GetActiveScene() const { return m_activeScene; }

		void Update(f32 deltaTime);
		void Render(Gfx::Renderer& renderer, Gfx::Camera& camera);

	private:
		const Gfx::ResourceFactory&                             m_resourceFactory;
		std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
		Scene*                                                  m_activeScene = nullptr;
	};
}