#pragma once
#include "Math/Math.h"
#include "Application/AppEvents.h"
#include "Graphics/Camera.h"
#include "Graphics/Model.h"
#include <Elos/Utils/Timer.h>

namespace Elos { class Window; }

namespace Prism
{
	namespace Gfx { class Renderer; }

	struct WVP
	{
		Matrix World;
		Matrix View;
		Matrix Projection;
	};

	class Scene
	{
	public:
		Scene(Elos::Timer* appTimer, Elos::Window* appWindow, AppEvents& appEvents, Gfx::Renderer* renderer);
		virtual ~Scene();

		NODISCARD inline Gfx::Camera* GetCamera() const noexcept { return m_camera.get(); }

		virtual void OnInit() = 0;
		virtual void OnTick(const Elos::Timer::TimeInfo& timeInfo);
		virtual void Render() = 0;
		virtual void RenderUI() = 0;
		virtual void OnShutdown() = 0;

	protected:
		void BindToAppEvents(AppEvents& appEvents);
		
		virtual void OnResize(const Elos::Event::Resized& e);
		virtual void OnKeyPressed(MAYBE_UNUSED const Elos::Event::KeyPressed& e) {}
		virtual void OnKeyReleased(MAYBE_UNUSED const Elos::Event::KeyReleased& e) {}
		virtual void OnMouseMovedRaw(MAYBE_UNUSED const Elos::Event::MouseMovedRaw& e) {}
		virtual void OnMouseButtonPressed(MAYBE_UNUSED const Elos::Event::MouseButtonPressed& e) {}
		virtual void OnMouseButtonReleased(MAYBE_UNUSED const Elos::Event::MouseButtonReleased& e) {}
		virtual void OnMouseWheelScrolled(MAYBE_UNUSED const Elos::Event::MouseWheelScrolled& e) {}

	protected:
		Gfx::Renderer*                           m_renderer;
		std::unique_ptr<Gfx::Camera>             m_camera;
		std::vector<std::shared_ptr<Gfx::Model>> m_models;

	private:
		Elos::Timer*        m_appTimer;
		Elos::Window*       m_appWindow;
		AppEventConnections m_connections;
	};
}