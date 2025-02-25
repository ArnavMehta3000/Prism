#pragma once
#include "StandardTypes.h"
#include "Graphics/Renderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Mesh.h"
#include "Graphics/Resources/Buffers/ConstantBuffer.h"
#include "Graphics/Resources/Shaders/Shader.h"
#include <Elos/Window/Window.h>

namespace Prism
{
	struct WVP
	{
		Matrix World;
		Matrix View;
		Matrix Projection;
	};

	class App
	{
	public:
		void Run();

	private:
		void Init();
		void Shutdown();
		void Tick();

		void CreateMainWindow();
		void ProcessWindowEvents();
		void CreateRenderer();
		void CreateResources();
		void CreateConstantBuffer();

	private:
		std::unique_ptr<class Elos::Window>       m_window;
		std::unique_ptr<Gfx::Renderer>            m_renderer;
		std::unique_ptr<Gfx::Camera>              m_camera;
		std::unique_ptr<Gfx::Mesh>                m_mesh;
		std::unique_ptr<Gfx::ConstantBuffer<WVP>> m_wvpCBuffer;
		std::unique_ptr<Gfx::Shader>              m_shaderVS;
		std::unique_ptr<Gfx::Shader>              m_shaderPS;

		bool m_hasFocus = false;
	};
}
