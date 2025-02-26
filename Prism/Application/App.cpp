#include "App.h"
#include "Application/Scene/SceneNodeFactory.h"
#include "Application/Scene/AnimatedSceneNode.h"
#include "Utils/Log.h"
#include "Graphics/Utils/ResourceFactory.h"
#include <DirectXColors.h>
#include <VertexTypes.h>
#include <Elos/Window/Utils/WindowExtensions.h>
#include <Elos/Common/Assert.h>
#include <array>
#include <chrono>

namespace Prism
{
	void App::Run()
	{
		Init();

		Log::Info("Starting App");

		auto lastTime = std::chrono::high_resolution_clock::now();

		while (m_window->IsOpen())
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();
			const f32 deltaTime = std::chrono::duration<f32>(currentTime - lastTime).count();
			lastTime = currentTime;
			
			ProcessWindowEvents();
			Tick(deltaTime);
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
		
		Log::Info("Creating resources");
		CreateResources();

		Log::Info("Creating constant buffer");
		CreateConstantBuffer();

		Log::Info("Setting up scene");
		SetupScene();
	}

	void App::Shutdown()
	{
		m_sceneManager.reset();
		m_wvpCBuffer.reset();
		m_shaderVS.reset();
		m_shaderPS.reset();
		m_mesh.reset();
		m_camera.reset();
		m_renderer.reset();
		m_window.reset();
	}

	void App::Tick(const f32 deltaTime)
	{
		m_sceneManager->Update(deltaTime);

		m_camera->SetLookAt(Vector3::Zero);
		m_camera->Update();
		
		m_renderer->ClearState();
		m_renderer->ClearBackBuffer(DirectX::Colors::CadetBlue);
		m_renderer->ClearDepthStencilBuffer(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);

		m_renderer->SetBackBufferRenderTarget();
		m_renderer->SetWindowAsViewport();
		m_renderer->SetSolidRenderState();

		//m_sceneManager->Render(*m_renderer, *m_camera);

		DX11::IBuffer* const d3dCBuffer = m_wvpCBuffer->GetBuffer();
		m_renderer->SetConstantBuffers(0, Gfx::Shader::Type::Vertex, std::span(&d3dCBuffer, 1));

		// Update CB
		WVP wvp
		{
			.World      = Matrix::CreateTranslation(Vector3::Zero).Transpose(),  // Mesh location
			.View       = m_camera->GetViewMatrix().Transpose(),
			.Projection = m_camera->GetProjectionMatrix().Transpose()
		};
		
		if (auto result = m_renderer->UpdateConstantBuffer(*m_wvpCBuffer, wvp); !result)
		{
#ifdef PRISM_BUILD_DEBUG
			Elos::ASSERT(false).Msg("Failed to update constant buffer").Throw();
#endif
		}
		
		// Draw the mesh
		m_renderer->SetShader(*m_shaderVS);
		m_renderer->SetShader(*m_shaderPS);
		m_renderer->DrawMesh(*m_mesh);

		wvp.World = Matrix::CreateTranslation(Vector3(1.5f, 1.5f, 1.5f)).Transpose();
		std::ignore = m_renderer->UpdateConstantBuffer(*m_wvpCBuffer, wvp);
		m_renderer->DrawMesh(*m_mesh);

		m_renderer->Present();
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
					m_camera->ZoomBy(e.Delta * 0.05f);
					Log::Warn("Camera Zoom {}\t Wheel Delta: {}", m_camera->GetZoomLevel(), e.Delta);
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

		m_renderer = std::make_unique<Gfx::Renderer>(*m_window, deviceDesc, swapChainDesc, DXGI_FORMAT_R32_TYPELESS);

		// Create camera
		Gfx::Camera::CameraDesc cameraDesc;
		cameraDesc.Position    = Vector3(0, 0, 10);
		cameraDesc.AspectRatio = static_cast<f32>(windowSize.Width) / static_cast<f32>(windowSize.Height);
		cameraDesc.VerticalFOV = 60.0f;
		cameraDesc.OrthoWidth  = 20.0f;
		cameraDesc.OrthoHeight = 11.25f;
		cameraDesc.NearPlane = 0.01f;
		
		m_camera = std::make_unique<Gfx::Camera>(cameraDesc);
	}
	
	void App::CreateResources()
	{
		using namespace DirectX;

#pragma region Create mesh
		const std::array vertices =
		{
			VertexPositionColor(XMFLOAT3(-1, -1, -1), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)),
			VertexPositionColor(XMFLOAT3(1, -1, -1), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)),
			VertexPositionColor(XMFLOAT3(1, 1, -1), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)),
			VertexPositionColor(XMFLOAT3(-1, 1, -1), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)),
			VertexPositionColor(XMFLOAT3(-1, -1, 1), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)),
			VertexPositionColor(XMFLOAT3(1, -1, 1), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)),
			VertexPositionColor(XMFLOAT3(1, 1, 1), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)),
			VertexPositionColor(XMFLOAT3(-1, 1, 1), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)),
		};
		
		constexpr std::array<u32, 36> indices =
		{
			0, 1, 3, 3, 1, 2,
			1, 5, 2, 2, 5, 6,
			5, 4, 6, 6, 4, 7,
			4, 0, 7, 7, 0, 3,
			3, 2, 7, 7, 2, 6,
			4, 5, 0, 0, 5, 1
		};

		Gfx::Mesh::MeshDesc meshDesc{};
		meshDesc.VertexStride = sizeof(VertexPositionColor);

		const auto& resourceFactory = m_renderer->GetResourceFactory();

		if (auto meshResult = resourceFactory.CreateMesh<VertexPositionColor>(vertices, indices, meshDesc); !meshResult)
		{
			Elos::ASSERT(SUCCEEDED(meshResult.error().ErrorCode)).Msg("Failed to create mesh! (Error Code: {:#x})", meshResult.error().ErrorCode).Throw();
		}
		else
		{
			m_mesh = std::move(meshResult.value());
		}
#pragma endregion


#pragma region Create shaders
		if (auto shaderResult = resourceFactory.CreateShader<Gfx::Shader::Type::Vertex>("Shaders/DrawCube_VS.cso"); !shaderResult)
		{
			Elos::ASSERT(SUCCEEDED(shaderResult.error().ErrorCode)).Msg("Failed to create vertex shader! (Error Code: {:#x})", shaderResult.error().ErrorCode).Throw();
		}
		else
		{
			m_shaderVS = std::move(shaderResult.value());
			m_shaderVS->SetShaderDebugName("Test_VS");
		}

		if (auto shaderResult = resourceFactory.CreateShader<Gfx::Shader::Type::Pixel>("Shaders/DrawCube_PS.cso"); !shaderResult)
		{
			Elos::ASSERT(SUCCEEDED(shaderResult.error().ErrorCode)).Msg("Failed to create pixel shader! (Error Code: {:#x})", shaderResult.error().ErrorCode).Throw();
		}
		else
		{
			m_shaderPS = std::move(shaderResult.value());
			m_shaderPS->SetShaderDebugName("Test_PS");
		}
#pragma endregion

#if PRISM_BUILD_DEBUG  // Shader pointers will be valid here. The above asserts should catch them
		Elos::ASSERT(Gfx::Shader::IsValid(*m_shaderVS)).Msg("Vertex shader not valid!").Throw();
		Elos::ASSERT(Gfx::Shader::IsValid(*m_shaderPS)).Msg("Pixel shader not valid!").Throw();
#endif
	}
	
	void App::CreateConstantBuffer()
	{
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
	
	void App::SetupScene()
	{
		m_sceneManager = std::make_unique<SceneManager>(m_renderer->GetResourceFactory());

		Scene* mainScene = m_sceneManager->CreateScene("MainScene");

		if (!mainScene->Init(*m_renderer))
		{
			Log::Error("Failed to initialize main scene!");
			return;
		}

		mainScene->SetVertexShader(m_shaderVS.get());
		mainScene->SetPixelShader(m_shaderPS.get());

		///SceneNodeFactory::CreateSolarSystem(*mainScene, m_mesh);





		auto complexNode = std::make_unique<AnimatedSceneNode>("Complex");
		AnimatedSceneNode* nodePtr = complexNode.get();

		// Get standard animation functions
		auto rotateAnim = AnimatedSceneNode::CreateRotationAnimation(0.5f, Vector3::Up);
		auto oscillateAnim = AnimatedSceneNode::CreateOscillationAnimation(1.0f, 0.5f, Vector3::Right);

		// Combine animations
		auto combinedAnim = [rotateAnim, oscillateAnim](AnimatedSceneNode& node, float deltaTime) 
		{
			rotateAnim(node, deltaTime);
			oscillateAnim(node, deltaTime);
		};

		// Set the combined animation
		SceneNode::RenderData rd{ .Mesh = m_mesh };
		nodePtr->SetRenderData(rd);
		nodePtr->SetAnimationFunction(combinedAnim);
		mainScene->GetRootNode()->AddChild(std::move(complexNode));


		m_sceneManager->SetActiveScene("MainScene");
	}
}
