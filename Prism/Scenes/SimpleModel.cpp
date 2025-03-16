#include "SimpleModel.h"
#include "Application/AppEvents.h"
#include "Graphics/Renderer.h"
#include "Graphics/Utils/ResourceFactory.h"
#include "Utils/Log.h"
#include <Elos/Common/Assert.h>
#include <DirectXColors.h>
#include <imgui.h>

namespace Prism
{
	SimpleModelScene::SimpleModelScene(Elos::Timer* appTimer, Elos::Window* appWindow, AppEvents& appEvents, Gfx::Renderer* renderer)
		: Scene(appTimer, appWindow, appEvents, renderer)
	{
	}
	
	void SimpleModelScene::OnInit()
	{
		LoadModel();
		LoadShaders();
		LoadBuffers();
	}
	
	void SimpleModelScene::OnTick(const Elos::Timer::TimeInfo& timeInfo)
	{
		Scene::OnTick(timeInfo);
		//const f32 radians = DirectX::XMConvertToRadians(static_cast<f32>(timeInfo.TotalTime) * 50.0f);
		constexpr const f32 angle = DirectX::XMConvertToRadians(90.0f);
		const Matrix world = Matrix::CreateRotationZ(angle) * Matrix::CreateRotationY(angle) * Matrix::CreateRotationX(angle);

		WVP wvp
		{
			.World      = world.Transpose(),
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
	
	void SimpleModelScene::Render()
	{
		m_renderer->BeginEvent(L"Clear Back Buffers");
		m_renderer->ClearState();
		m_renderer->ClearBackBuffer(DirectX::Colors::CadetBlue);
		m_renderer->ClearDepthStencilBuffer(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
		m_renderer->EndEvent();

		m_renderer->BeginEvent(L"Set Resources");
		m_renderer->SetBackBufferRenderTarget();
		m_renderer->SetWindowAsViewport();
		
		Gfx::Buffer* wvpBuffers[] = { m_wvpCBuffer.get() };
		m_renderer->SetConstantBuffers(0, Gfx::Shader::Type::Vertex, std::span{ wvpBuffers });
		m_renderer->SetShader(*m_shaderVS);
		m_renderer->SetShader(*m_shaderPS);
		m_renderer->EndEvent();

		// Draw the model
		m_renderer->BeginEvent(L"Draw model");
		m_model->Render(*m_renderer);
		m_renderer->EndEvent();
	}
	
	void SimpleModelScene::RenderUI()
	{
		Scene::RenderUI();
	}
	
	void SimpleModelScene::OnShutdown()
	{
		m_wvpCBuffer.reset();
		m_shaderVS.reset();
		m_shaderPS.reset();
		m_model.reset();
	}
	
	void SimpleModelScene::LoadModel()
	{
		bool success = false;

		Elos::ScopedTimer loadTimer([&success](const Elos::Timer::TimeInfo& timeInfo)
		{
			if (success)
			{
				Log::Info("Imported mesh {} in {:3f}s", AssetPath.data(), timeInfo.TotalTime);
			}
		});

		const auto& resourceFactory = m_renderer->GetResourceFactory();

		Prism::Gfx::MeshImporter::ImportSettings settings{};
		settings.FlipUVs = false;

		if (auto modelResult = Gfx::Model::LoadFromFile(resourceFactory, AssetPath, settings); modelResult)
		{
			m_model = modelResult.value();
			success = true;
		}
		else
		{
			Elos::ASSERT(false).Msg("{}", modelResult.error().Message).Throw();
		}
	}
	
	void SimpleModelScene::LoadShaders()
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
	
	void SimpleModelScene::LoadBuffers()
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