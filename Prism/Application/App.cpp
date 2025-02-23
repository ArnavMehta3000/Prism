#include "App.h"
#include "Utils/Log.h"
#include <DirectXColors.h>
#include <Elos/Window/Utils/WindowExtensions.h>
#include <Elos/Common/Assert.h>
#include <array>

namespace Prism
{
	void App::Run()
	{
		Init();

		Log::Info("Starting App");

		while (m_window->IsOpen())
		{
			ProcessWindowEvents();
			Tick();
		}
		Log::Info("Stopping App");

		Shutdown();
	}

	void App::Init()
	{
		Log::Info("Initializing application");
		CreateMainWindow();


		Log::Info("Initializing renderer");
		CreateRenderer();
	}

	void App::Shutdown()
	{
		m_renderer.reset();
		m_window.reset();
	}

	void App::Tick()
	{
		if (m_camera)
		{
			m_camera->Update();
		}

		if (m_renderer)
		{
			m_renderer->ClearBackBuffer(DirectX::Colors::CadetBlue);
			m_renderer->Present();
		}
	}

	void App::CreateMainWindow()
	{
		m_window = std::make_unique<Elos::Window>(
			Elos::WindowCreateInfo::Default("Prism", { 1280, 720 }));

		Elos::ASSERT(m_window && m_window->GetHandle())
			.Msg("Failed to create window")
			.Throw();

		Elos::WindowExtensions::EnableDarkMode(m_window->GetHandle(), true);
		m_window->SetMinimumSize({ 600, 400 });
	}

	void App::ProcessWindowEvents()
	{
		const auto OnWindowClosedEvent = [this](const Elos::Event::Closed&)
		{
			m_window->Close();
		};

		const auto OnWindowKeyReleased  = [this](const Elos::Event::KeyPressed& e)
		{
			if (e.Key == Elos::KeyCode::Escape)
			{
				Log::Info("Escape pressed...Closing window");
				m_window->Close();
			}
		};

		const auto OnWindowResizedEvent = [this](const Elos::Event::Resized& e)
		{
			Log::Info("Window resized: {0}x{1}", e.Size.Width, e.Size.Height);

			if (m_camera)
			{
				m_camera->Resize(e.Size.Width, e.Size.Height);
				Log::Info("Camera aspect ratio: {}", m_camera->GetAspectRatio());
			}

			if (m_renderer)
			{
				m_renderer->Resize(e.Size.Width, e.Size.Height);
			}
		};


		m_window->HandleEvents(
			OnWindowClosedEvent,
			OnWindowResizedEvent,
			OnWindowKeyReleased
		);
	}

	void App::CreateRenderer()
	{
		const Gfx::Core::Device::DeviceDesc deviceDesc
		{
			.EnableDebugLayer = true,
		};

		const Elos::WindowSize windowSize = m_window->GetSize();

		const Gfx::Core::SwapChain::SwapChainDesc swapChainDesc
		{
			.WindowHandle = m_window->GetHandle(),
			.Width        = windowSize.Width,
			.Height       = windowSize.Height,
			.BufferCount  = 2,
			.Format       = DXGI_FORMAT_R8G8B8A8_UNORM,
			.AllowTearing = true,
			.Fullscreen   = false
		};

		m_renderer = std::make_unique<Gfx::Renderer>(*m_window, deviceDesc, swapChainDesc);

		// Create camera
		Gfx::Camera::CameraDesc cameraDesc;
		cameraDesc.Position    = Vector3(0, 5, -10);
		cameraDesc.AspectRatio = static_cast<f32>(windowSize.Width) / static_cast<f32>(windowSize.Height);
		cameraDesc.VerticalFOV = 60.0f;
		cameraDesc.OrthoWidth  = 20.0f;
		cameraDesc.OrthoHeight = 11.25f;
		m_camera = std::make_unique<Gfx::Camera>(cameraDesc);
	}
}
