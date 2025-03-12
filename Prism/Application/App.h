#pragma once
#include "StandardTypes.h"
#include "Application/WVP.h"
#include "Scene/SceneGraph.h"
#include "Graphics/Renderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Mesh.h"
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
		void CreateResources();
		void CreateConstantBuffer();
		void CreateScene();

	private:
		std::shared_ptr<class Elos::Window>       m_window;
		std::shared_ptr<Gfx::Renderer>            m_renderer;
		std::shared_ptr<Gfx::Camera>              m_camera;

		std::shared_ptr<Gfx::Mesh>                m_mesh;
		std::shared_ptr<Gfx::ConstantBuffer<WVP>> m_wvpCBuffer;
		std::shared_ptr<Gfx::Shader>              m_shaderVS;
		std::shared_ptr<Gfx::Shader>              m_shaderPS;
		SceneGraph                                m_sceneGraph;
		Elos::Timer                               m_timer;

		bool m_isMouseDown = false;
	};
}
