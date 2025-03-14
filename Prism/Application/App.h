#pragma once
#include "Application/Scene.h"
#include "Graphics/Renderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Model.h"
#include "Graphics/Resources/Buffers/ConstantBuffer.h"
#include "Graphics/Resources/Shaders/Shader.h"
#include <Elos/Window/Window.h>
#include <Elos/Utils/Timer.h>

namespace Prism
{
	class App
	{
	public:
		void Run();

	private:
		void Init();
		void Shutdown();
		void Tick(const Elos::Timer::TimeInfo& timeInfo);
		void Render();

		void CreateMainWindow();
		void ProcessWindowEvents();
		void CreateRenderer();
		void CreateShaders();
		void LoadGLTF();
		void CreateConstantBuffer();
		void InitImGui();
		void ShutdownImGui();

	private:
		bool                                      m_isMouseDown = false;
		bool                                      m_isSolidRenderState = true;
		std::shared_ptr<class Elos::Window>       m_window;
		std::shared_ptr<Gfx::Renderer>            m_renderer;
		std::shared_ptr<Gfx::Camera>              m_camera;
		std::shared_ptr<Prism::Gfx::Model>        m_model;
		std::shared_ptr<Gfx::ConstantBuffer<WVP>> m_wvpCBuffer;
		std::shared_ptr<Gfx::Shader>              m_shaderVS;
		std::shared_ptr<Gfx::Shader>              m_shaderPS;
		Elos::Timer                               m_timer;
	};
}
