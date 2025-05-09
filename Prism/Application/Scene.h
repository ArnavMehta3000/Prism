#pragma once
#include "Application/CommonTypes.h"
#include "Application/AppEvents.h"
#include "Application/CameraController.h"
#include "Graphics/Model.h"
#include <Elos/Utils/Timer.h>

namespace Elos { class Window; }

namespace Prism
{
	namespace Gfx { class Renderer; }

	class Scene
	{
	public:
		Scene(Elos::Timer* appTimer, Elos::Window* appWindow, AppEvents& appEvents, Gfx::Renderer* renderer);
		virtual ~Scene();

		NODISCARD inline Gfx::Camera* GetCamera() const noexcept { return m_camera.get(); }

		virtual void OnInit() = 0;
		virtual void OnTick(const Elos::Timer::TimeInfo& timeInfo);
		virtual void Render() = 0;
		virtual void RenderUI();
		virtual void OnShutdown() = 0;

	protected:
		void BindToAppEvents(AppEvents& appEvents);
		
		virtual void OnResize(const Elos::Event::Resized& e);
		virtual void OnKeyPressed(const Elos::Event::KeyPressed& e);
		virtual void OnKeyReleased(const Elos::Event::KeyReleased& e);
		virtual void OnMouseMovedRaw(const Elos::Event::MouseMovedRaw& e);
		virtual void OnMouseButtonPressed(const Elos::Event::MouseButtonPressed& e);
		virtual void OnMouseButtonReleased(const Elos::Event::MouseButtonReleased& e);
		virtual void OnMouseWheelScrolled(const Elos::Event::MouseWheelScrolled& e);

	private:
		void RenderCameraDebugOverlay();
		void RenderCameraControlsWindow();

	protected:
		Gfx::Renderer*                           m_renderer;
		std::unique_ptr<Gfx::Camera>             m_camera;
		std::unique_ptr<CameraController>        m_cameraController;
		std::vector<std::shared_ptr<Gfx::Model>> m_models;

	private:
		Elos::Timer*        m_appTimer;
		Elos::Window*       m_appWindow;
		AppEventConnections m_connections;
	};
}