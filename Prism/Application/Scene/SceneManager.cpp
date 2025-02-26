#include "SceneManager.h"
#include "Graphics/Renderer.h"
#include "Graphics/Camera.h"
#include "Utils/Log.h"
#include <Elos/Common/Assert.h>

namespace Prism
{
	SceneManager::SceneManager(const Gfx::ResourceFactory& resourceFactory)
		: m_resourceFactory(resourceFactory)
	{
		Log::Info("SceneManager initialized");
	}
	
	SceneManager::~SceneManager()
	{
		Log::Info("SceneManager shutting down");
		m_activeScene = nullptr;
		m_scenes.clear();
	}
	
	Scene* SceneManager::CreateScene(const std::string& name)
	{
		// Check if a scene with this name already exists
		if (m_scenes.find(name) != m_scenes.end())
		{
			Log::Warn("Scene with name '{}' already exists", name);
			return m_scenes[name].get();
		}

		auto scene = std::make_unique<Scene>(name);
		Scene* scenePtr = scene.get();

		// Initialize the scene
		// Note: In a full implementation, we'd need access to the renderer here
		// For now, the App class will handle initialization

		// Store the scene
		m_scenes[name] = std::move(scene);

		// If this is our first scene, make it active
		if (!m_activeScene)
		{
			m_activeScene = scenePtr;
			Log::Info("Scene '{}' set as active (first scene)", name);
		}

		return scenePtr;
	}

	Scene* SceneManager::LoadScene(const std::string& name)
	{
		// For now, just create an empty scene
		return CreateScene(name);
	}
	
	void SceneManager::DestroyScene(const std::string& name)
	{
		auto it = m_scenes.find(name);
		if (it != m_scenes.end())
		{
			// If we're destroying the active scene, clear the active scene pointer
			if (m_activeScene == it->second.get())
			{
				m_activeScene = nullptr;
				Log::Info("Active scene '{}' was destroyed", name);
			}

			// Remove the scene
			m_scenes.erase(it);
			Log::Info("Scene '{}' destroyed", name);
		}
	}
	
	Scene* SceneManager::GetScene(const std::string& name) const
	{
		auto it = m_scenes.find(name);
		return (it != m_scenes.end()) ? it->second.get() : nullptr;
	}
	
	void SceneManager::SetActiveScene(const std::string& name)
	{
		auto it = m_scenes.find(name);
		if (it != m_scenes.end())
		{
			m_activeScene = it->second.get();
			Log::Info("Scene '{}' set as active", name);
		}
		else
		{
			Log::Warn("Cannot set scene '{}' as active: scene not found", name);
		}
	}
	
	void SceneManager::Update(f32 deltaTime)
	{
		if (m_activeScene)
		{
			m_activeScene->Update(deltaTime);
		}
	}
	
	void SceneManager::Render(Gfx::Renderer& renderer, Gfx::Camera& camera)
	{
		if (m_activeScene)
		{
			m_activeScene->Render(renderer, camera);
		}
	}
}