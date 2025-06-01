#include "SimpleModel.h"
#include "Application/AppEvents.h"
#include "Application/Globals.h"
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
		LoadSampler();
	}
	
	void SimpleModelScene::OnTick(const Elos::Timer::TimeInfo& timeInfo)
	{
		Scene::OnTick(timeInfo);

		const WVP wvp
		{
			.World      = m_model->GetTransform().GetTransposedWorldMatrix(),
			.View       = m_camera->GetViewMatrix().Transpose(),
			.Projection = m_camera->GetProjectionMatrix().Transpose()
		};

		std::ignore = m_renderer->UpdateConstantBuffer(*m_wvpCBuffer, wvp);
	}
	
	void SimpleModelScene::Render()
	{
		m_renderer->BeginEvent(L"Frame");
		{

			m_renderer->BeginEvent(L"Clear Back Buffers");
			{
				m_renderer->ClearState();
				m_renderer->ClearBackBuffer(DirectX::Colors::CadetBlue);
				m_renderer->ClearDepthStencilBuffer(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
			}
			m_renderer->EndEvent();

			m_renderer->BeginEvent(L"Set Resources");
			{
				m_renderer->SetBackBufferRenderTarget();
				m_renderer->SetWindowAsViewport();

				Gfx::Buffer* wvpBuffers[] = { m_wvpCBuffer.get() };
				m_renderer->SetConstantBuffers(0, Gfx::Shader::Type::Vertex, std::span{ wvpBuffers });
				m_renderer->SetShader(*m_shaderVS);
				m_renderer->SetShader(*m_shaderPS);
				
				DX11::ISamplerState* samplers[] = { m_linearSampler.Get() };
				m_renderer->SetSamplerState(Gfx::Shader::Type::Pixel, 0, std::span{ samplers });
			}
			m_renderer->EndEvent();

			// Draw the model
			m_renderer->BeginEvent(L"Draw model");
			{
				m_model->Render(*m_renderer);
			}
			m_renderer->EndEvent();
		}
		m_renderer->EndEvent();
	}
	
	void SimpleModelScene::RenderUI()
	{
		Scene::RenderUI();

		if (ImGui::Begin("Model Controls"))
		{
			Transform& transform = m_model->GetTransform();

			bool modified = false;
			modified |= ImGui::DragFloat3("Position", &transform.Position.x, 0.1f);
			modified |= ImGui::DragFloat3("Rotation", &transform.Rotation.x, 0.1f);
			modified |= ImGui::DragFloat3("Scale", &transform.Scale.x, 0.1f);
			
			if (modified)
			{
				transform.UpdateWorldMatrix();
			}
			
			ImGui::DragInt("Texture", &Globals::g_textureNumber, 1, 0, m_model->GetTextures().size() - 1);
		}
		ImGui::End();
	}
	
	void SimpleModelScene::OnShutdown()
	{
		m_linearSampler.Reset();
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
	
	void SimpleModelScene::LoadSampler()
	{
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Linear filtering for min, mag, and mip
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;    // Wrap addressing mode for U coordinate
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;    // Wrap addressing mode for V coordinate
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;    // Wrap addressing mode for W coordinate
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;  // No comparison
		sampDesc.MinLOD = 0;                               // Minimum mip level
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;               // Maximum mip level

		HRESULT hr = m_renderer->GetDevice()->GetDevice()->CreateSamplerState(&sampDesc, &m_linearSampler);
		if (FAILED(hr))
		{
			// Handle error
		}

	}
}