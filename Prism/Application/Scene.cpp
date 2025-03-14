#include "Scene.h"
#include <Elos/Window/Window.h>
#include <Elos/Common/Assert.h>

namespace Prism
{
	Scene::Scene(Elos::Timer* appTimer, Elos::Window* appWindow, AppEvents& appEvents, Gfx::Renderer* renderer)
		: m_appTimer(appTimer)
		, m_appWindow(appWindow)
		, m_renderer(renderer)
	{
#ifdef PRISM_BUILD_DEBUG
		Elos::ASSERT(m_appTimer && m_appWindow && m_renderer)
			.Msg("Failed to create scene. One or more input pointers are nullptr")
			.Throw();
#endif

		// Create camera
		const Elos::WindowSize windowSize = m_appWindow->GetSize();
		Gfx::Camera::CameraDesc cameraDesc;
		cameraDesc.AspectRatio = static_cast<f32>(windowSize.Width) / static_cast<f32>(windowSize.Height);
		m_camera = std::make_unique<Gfx::Camera>(cameraDesc);

		// Bind scene events
		BindToAppEvents(appEvents);
	}

	Scene::~Scene()
	{
		m_connections.DisconnectAll();
		m_camera.reset();
	}

	void Scene::OnTick(const Elos::Timer::TimeInfo& timeInfo)
	{
		m_camera->Update();
	}

	void Scene::OnResize(const Elos::Event::Resized& e)
	{
		m_camera->Resize(e.Size.Width, e.Size.Height);
	}
	
	void Scene::BindToAppEvents(AppEvents& appEvents)
	{
		m_connections.OnResized             = appEvents.OnResized.Connect([this](const auto& e) { OnResize(e); });
		m_connections.OnKeyPressed          = appEvents.OnKeyPressed.Connect([this](const auto& e) { OnKeyPressed(e); });
		m_connections.OnKeyReleased         = appEvents.OnKeyReleased.Connect([this](const auto& e) { OnKeyReleased(e); });
		m_connections.OnMouseButtonPressed  = appEvents.OnMouseButtonPressed.Connect([this](const auto& e) { OnMouseButtonPressed(e); });
		m_connections.OnMouseButtonReleased = appEvents.OnMouseButtonReleased.Connect([this](const auto& e) { OnMouseButtonReleased(e); });
		m_connections.OnMouseMovedRaw       = appEvents.OnMouseMovedRaw.Connect([this](const auto& e) { OnMouseMovedRaw(e); });
		m_connections.OnMouseWheelScrolled  = appEvents.OnMouseWheelScrolled.Connect([this](const auto& e) { OnMouseWheelScrolled(e); });
	}
}
