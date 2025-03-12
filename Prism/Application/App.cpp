#include "App.h"
#include "Utils/Log.h"
#include "Graphics/Utils/ResourceFactory.h"
#include "Graphics/Primitives.h"
#include <Elos/Window/Utils/WindowExtensions.h>
#include <Elos/Common/Assert.h>
#include <Elos/Event/Signal.h>
#include <DirectXColors.h>

namespace Prism
{
	Elos::Signal<const Elos::Timer::TimeInfo&> g_perSecondEvent;	
	Elos::Connection<const Elos::Timer::TimeInfo&> g_updateWindowTitleConnection;


	void App::Run()
	{
		Init();
		Log::Info("Starting App");
		
		f64 prevTime = 0.0f;
		constexpr f64 updateInterval = 1.0f;
		
		// Using a signal to update the window title every second just for fun :)
		g_updateWindowTitleConnection = g_perSecondEvent.Connect([this](const Elos::Timer::TimeInfo& timeInfo)
		{
			m_window->SetTitle(
				std::format("Prism | FPS: {} | Frame Count: {} | Delta Time: {:.7f}",
					timeInfo.FrameCount,
					timeInfo.FPS,
					timeInfo.DeltaTime));
			
		});

		// Emit signal before starting
		g_perSecondEvent.Emit(Elos::Timer::TimeInfo{});

		while (m_window->IsOpen())
		{
			ProcessWindowEvents();
			
			m_timer.Tick([this, &prevTime](const Elos::Timer::TimeInfo& timeInfo)
			{
				// Update window title with FPS info every second
				prevTime += timeInfo.DeltaTime;
				if (prevTime >= updateInterval)
				{
					prevTime = 0.0f;
					g_perSecondEvent.Emit(timeInfo);
				}

				this->Tick(timeInfo);
			});

			Render();
		}

		g_updateWindowTitleConnection.Disconnect();

		Log::Info("Stopping App");
		Shutdown();
	}

	void App::Init()
	{
		Elos::ScopedTimer initTimer([](auto timeInfo) { Log::Info("Initialized app in {}s", timeInfo.TotalTime); });

		CreateMainWindow();
		CreateRenderer();
		CreateShaders();
		LoadGLTF();
		CreateConstantBuffer();
	}

	void App::Shutdown()
	{
		Elos::ScopedTimer initTimer([](auto timeInfo) { Log::Info("Shutdown app in {}s", timeInfo.TotalTime); });

		m_wvpCBuffer.reset();
		m_shaderVS.reset();
		m_shaderPS.reset();
		m_model.reset();
		m_camera.reset();
		m_renderer.reset();
		m_window.reset();
	}

	void App::Tick(MAYBE_UNUSED const Elos::Timer::TimeInfo& timeInfo)
	{
		m_camera->SetLookAt(Vector3::Zero);
		m_camera->Update();

		WVP wvp
		{
			.World      = Matrix::CreateTranslation(Vector3::Zero).Transpose(),  // Model location
			.View       = m_camera->GetViewMatrix().Transpose(),
			.Projection = m_camera->GetProjectionMatrix().Transpose()
		};

		if (auto result = m_renderer->UpdateConstantBuffer(*m_wvpCBuffer, wvp); !result)
		{
#ifdef PRISM_BUILD_DEBUG
			Elos::ASSERT(false).Msg("Failed to update constant buffer").Throw();
#endif
		}
	}

	void App::Render()
	{
		m_renderer->BeginEvent(L"Clear");
		m_renderer->ClearState();
		m_renderer->ClearBackBuffer(DirectX::Colors::CadetBlue);
		m_renderer->ClearDepthStencilBuffer(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
		m_renderer->EndEvent();

		m_renderer->BeginEvent(L"Set Resources");
		m_renderer->SetBackBufferRenderTarget();
		m_renderer->SetWindowAsViewport();

		if (m_isSolidRenderState)
		{
			m_renderer->SetSolidRenderState();
		}
		else
		{
			m_renderer->SetWireframeRenderState();
		}

		Gfx::Buffer* wvpBuffers[] = { m_wvpCBuffer.get() };
		m_renderer->SetConstantBuffers(0, Gfx::Shader::Type::Vertex, std::span{ wvpBuffers });
		m_renderer->SetShader(*m_shaderVS);
		m_renderer->SetShader(*m_shaderPS);
		m_renderer->EndEvent();

		// Draw the model

		m_renderer->BeginEvent(L"Draw model");
		m_model->Render(*m_renderer);
		m_renderer->EndEvent();

		m_renderer->Present();
	}

	void App::CreateMainWindow()
	{
		Elos::ScopedTimer initTimer([](auto timeInfo) { Log::Info("Created main window in {}s", timeInfo.TotalTime); });

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

			if (e.Key == Elos::KeyCode::R)
			{
				Log::Info("Resetting camera");
				m_camera->SetPosition(Vector3(0, 0, 10));
				m_camera->SetOrientation(Quaternion::Identity);
			}

			if (e.Key == Elos::KeyCode::C)
			{
				const auto currentProjType = m_camera->GetProjectionType();
				const auto nextProjType = currentProjType == Gfx::Camera::ProjectionType::Perspective ?
					Gfx::Camera::ProjectionType::Orthographic : Gfx::Camera::ProjectionType::Perspective;

				m_camera->SetProjectionType(nextProjType);
			}

			if (e.Key == Elos::KeyCode::W)
			{
				m_isSolidRenderState = !m_isSolidRenderState;
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

		const auto OnWindowMouseMoveRaw = [this](const Elos::Event::MouseMovedRaw& e)
		{
			if (m_camera && m_isMouseDown)
			{
				m_camera->RotateAround(Vector3::Zero, Vector3::Up, e.DeltaX * 0.01f);
				m_camera->RotateAround(Vector3::Zero, Vector3::Right, e.DeltaY * 0.01f);
			}
		};

		const auto OnWindowMousePressed = [this](const Elos::Event::MouseButtonPressed& e)
		{
			if (e.Button == Elos::KeyCode::MouseButton::Left)
			{
				m_isMouseDown = true;
			}
		};

		const auto OnWindowMouseReleased = [this](const Elos::Event::MouseButtonReleased& e)
		{
			if (e.Button == Elos::KeyCode::MouseButton::Left)
			{
				m_isMouseDown = false;
			}
		};

		const auto OnWindowMouseWheel = [this](const Elos::Event::MouseWheelScrolled& e)
		{
			if (e.Wheel == Elos::KeyCode::MouseWheel::Vertical)
			{
				if (m_camera)
				{
					m_camera->ZoomBy(e.Delta);
				}
			}
		};


		m_window->HandleEvents(
			OnWindowClosedEvent,
			OnWindowResizedEvent,
			OnWindowKeyReleased,
			OnWindowMouseMoveRaw,
			OnWindowMousePressed,
			OnWindowMouseReleased,
			OnWindowMouseWheel
		);
	}

	void App::CreateRenderer()
	{
		Elos::ScopedTimer initTimer([](auto timeInfo) { Log::Info("Initialized renderer in {}s", timeInfo.TotalTime); });

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
			.SyncInterval = 0,  // No vsync
			.Format       = DXGI_FORMAT_R8G8B8A8_UNORM,
			.AllowTearing = true,
			.Fullscreen   = false
		};

		m_renderer = std::make_unique<Gfx::Renderer>(*m_window, deviceDesc, swapChainDesc, DXGI_FORMAT_R32_TYPELESS);

		// Create camera
		Gfx::Camera::CameraDesc cameraDesc;
		cameraDesc.AspectRatio = static_cast<f32>(windowSize.Width) / static_cast<f32>(windowSize.Height);
		m_camera = std::make_unique<Gfx::Camera>(cameraDesc);
	}

	void App::CreateShaders()
	{
		Elos::ScopedTimer initTimer([](auto timeInfo) { Log::Info("Created resources in {}s", timeInfo.TotalTime); });
		using namespace DirectX;
		
		const auto& resourceFactory = m_renderer->GetResourceFactory();

		if (auto shaderResult = resourceFactory.CreateShader<Gfx::Shader::Type::Vertex>("Shaders/SimpleModel_VS.cso"); !shaderResult)
		{
			Elos::ASSERT(SUCCEEDED(shaderResult.error().ErrorCode)).Msg("Failed to create vertex shader! (Error Code: {:#x})", shaderResult.error().ErrorCode).Throw();
		}
		else
		{
			m_shaderVS = std::move(shaderResult.value());
			m_shaderVS->SetShaderDebugName("SimpleModel_VS");
		}

		if (auto shaderResult = resourceFactory.CreateShader<Gfx::Shader::Type::Pixel>("Shaders/SimpleModel_PS.cso"); !shaderResult)
		{
			Elos::ASSERT(SUCCEEDED(shaderResult.error().ErrorCode)).Msg("Failed to create pixel shader! (Error Code: {:#x})", shaderResult.error().ErrorCode).Throw();
		}
		else
		{
			m_shaderPS = std::move(shaderResult.value());
			m_shaderPS->SetShaderDebugName("SimpleModel_PS");
		}

#if PRISM_BUILD_DEBUG  // Shader pointers will be valid here. The above asserts should catch them
		Elos::ASSERT(Gfx::Shader::IsValid(*m_shaderVS)).Msg("Vertex shader not valid!").Throw();
		Elos::ASSERT(Gfx::Shader::IsValid(*m_shaderPS)).Msg("Pixel shader not valid!").Throw();
#endif
	}

	void App::LoadGLTF()
	{
		constexpr auto assetPath = PRISM_ASSETS_PATH "/DamagedHelmet.gltf";
		bool success = false;

		Elos::ScopedTimer loadTimer([&assetPath, &success](const Elos::Timer::TimeInfo& timeInfo)
		{
			if (success)
			{
				Log::Info("Imported mesh {} in {:3f}s", assetPath, timeInfo.TotalTime);
			}
		});

		const auto& resourceFactory = m_renderer->GetResourceFactory();

		Prism::Gfx::MeshImporter::ImportSettings settings{};
		settings.FlipUVs = false;


		auto modelResult = Gfx::Model::LoadFromFile(resourceFactory, assetPath, settings);
		if (modelResult)
		{
			m_model = modelResult.value();
			success = true;
		}
	}

	void App::CreateConstantBuffer()
	{
		Elos::ScopedTimer initTimer([](auto timeInfo) { Log::Info("Created constant buffer in {}s", timeInfo.TotalTime); });
		const auto& resourceFactory = m_renderer->GetResourceFactory();

		if (auto cbResult = resourceFactory.CreateConstantBuffer<WVP>(); !cbResult)
		{
			Elos::ASSERT(SUCCEEDED(cbResult.error().ErrorCode)).Msg("Failed to create constant buffer! (Error Code: {:#x})", cbResult.error().ErrorCode).Throw();
		}
		else
		{
			m_wvpCBuffer = std::move(cbResult.value());
		}
	}
}
