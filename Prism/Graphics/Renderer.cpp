#include "Renderer.h"
#include "Utils/Log.h"
#include "Graphics/Utils/ResourceFactory.h"
#include "Graphics/Camera.h"
#include <Elos/Common/Assert.h>
#include <Elos/Window/Window.h>

namespace Prism::Gfx
{
	namespace Internal
	{
		void LogAdapterInfo(const Prism::Gfx::Core::AdapterInfo& adapterInfo)
		{
			Log::Info("Adapter Name: {}"                           , adapterInfo.Description);
			Log::Info("Adapter Vendor: {}"                         , adapterInfo.DxgiDesc.VendorId);
			Log::Info("Adapter Device: {}"                         , adapterInfo.DxgiDesc.DeviceId);
			Log::Info("Adapter Subsystem: {}"                      , adapterInfo.DxgiDesc.SubSysId);
			Log::Info("Adapter Revision: {}"                       , adapterInfo.DxgiDesc.Revision);
			Log::Info("Adapter Dedicated Video Memory(GB): {:.3f}" , static_cast<f32>(adapterInfo.DedicatedVideoMemory) / 1024 / 1024 / 1024);
			Log::Info("Adapter Shared System Memory(GB): {:.3f}"   , static_cast<f32>(adapterInfo.SharedSystemMemory) / 1024 / 1024 / 1024);
			Log::Info("Adapter Dedicated System Memory(GB): {:.3f}", static_cast<f32>(adapterInfo.DedicatedSystemMemory) / 1024 / 1024 / 1024);
		}
	}

	Renderer::Renderer(Elos::Window& window, const Core::Device::DeviceDesc& deviceDesc, const Core::SwapChain::SwapChainDesc& swapChainDesc)
		: m_window(window)
	{
		CreateDevice(deviceDesc);
		CreateSwapChain(swapChainDesc);

		m_resourceFactory = std::make_unique<ResourceFactory>(m_device.get());

		Log::Info("Created Renderer");
	}

	Renderer::~Renderer()
	{
		Log::Info("Shutting down Renderer");
		m_resourceFactory.reset();
		m_swapChain.reset();
		m_device.reset();
	}

	void Renderer::ClearState() const
	{
		m_device->GetContext()->ClearState();
	}

	void Renderer::ClearBackBuffer(const f32* clearColor) const
	{
		DX11::IDeviceContext* const context = m_device->GetContext();
		DX11::IRenderTarget* const backBufferRTV = m_swapChain->GetBackBufferRTV();
		
		context->ClearRenderTargetView(backBufferRTV, clearColor);
	}

	void Renderer::SetViewports(const std::span<D3D11_VIEWPORT> viewports) const
	{
		m_device->GetContext()->RSSetViewports(static_cast<u32>(viewports.size()), viewports.data());
	}

	void Renderer::SetWindowAsViewport() const
	{
		const Elos::WindowSize size = m_window.GetSize();
		
		D3D11_VIEWPORT vp
		{ 
			.TopLeftX = 0.0f, 
			.TopLeftY = 0.0f, 
			.Width    = static_cast<f32>(size.Width),
			.Height   = static_cast<f32>(size.Height), 
			.MinDepth = 0.0f, 
			.MaxDepth = 1.0f 
		};
		
		SetViewports(std::span<D3D11_VIEWPORT>(&vp, 1));
	}

	void Renderer::Resize(const u32 width, const u32 height) const
	{
		if (width == 0 || height == 0)
		{
			return;  // We cannot resize
		}

		if (m_swapChain)
		{
			// Only check for resize failure in debug builds
#if PRISM_BUILD_DEBUG
			if (auto result = m_swapChain->Resize(width, height); !result)
			{
				Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("Failed to resize DXGI Swap Chain! (Error Code: {:#x})", result.error().ErrorCode).Throw();
			}
#else
			std::ignore = m_swapChain->Resize(width, height);
#endif
		}
	}

	void Renderer::Present() const
	{
		if (m_swapChain)
		{
			// Only check for resize failure in debug builds
#if PRISM_BUILD_DEBUG
			if (auto result = m_swapChain->Present(); !result)
			{
				Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("Failed to present DXGI Swap Chain! (Error Code: {:#x})", result.error().ErrorCode).Throw();
			}
#else
			std::ignore = m_swapChain->Present();
#endif
		}
	}

	void Renderer::Flush() const
	{
		m_device->GetContext()->Flush();
	}

	void Renderer::SetBackBufferRenderTarget() const
	{
		ID3D11RenderTargetView* const backBufferRTV = m_swapChain->GetBackBufferRTV();
		m_device->GetContext()->OMSetRenderTargets(1, &backBufferRTV, nullptr);
	}

	void Renderer::Draw(u32 vertexCount, u32 startIndex) const
	{
		m_device->GetContext()->Draw(vertexCount, startIndex);
	}

	void Renderer::DrawAuto() const
	{
		m_device->GetContext()->DrawAuto();
	}

	void Renderer::DrawIndexed(const u32 indexCount, const u32 startIndexLocation, const i32 baseVertexLocation) const
	{
		m_device->GetContext()->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}

	void Renderer::DrawIndexedInstanced(const u32 indexCountPerInstance, const u32 instanceCount, const u32 startIndexLocation, const i32 baseVertexLocation, const u32 startInstanceLocation) const
	{
		m_device->GetContext()->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	}

	void Renderer::DrawInstanced(const u32 vertexCountPerInstance, const u32 instanceCount, const u32 startVertexLocation, const u32 startInstanceLocation) const
	{
		m_device->GetContext()->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
	}

	void Renderer::DrawMesh(const Mesh& mesh, bool bindMesh) const
	{
		DX11::IDeviceContext* const context = m_device->GetContext();
		if (bindMesh)
		{
			mesh.Bind(context);
		}

		context->DrawIndexed(mesh.GetIndexBuffer()->IndexCount, 0, 0);
	}

	void Renderer::SetShader(const Shader& shader) const
	{
		DX11::IDeviceContext* const context = m_device->GetContext();
		const Shader::Type type = shader.GetType();

		switch (type)
		{
			using enum Shader::Type;

		case Vertex:
		{
			const Shader::VertexShaderData& vsData = shader.As<Shader::Type::Vertex>();
			context->VSSetShader(vsData.Shader.Get(), nullptr, 0);
			context->IASetInputLayout(vsData.Layout.Get());
			break;
		}
		
		case Pixel:
		{
			const Shader::PixelShaderData& psData = shader.As<Shader::Type::Pixel>();
			context->PSSetShader(psData.Shader.Get(), nullptr, 0);
			break;
		}
		
		case Compute:
		{
			const Shader::ComputeShaderData& csData = shader.As<Shader::Type::Compute>();
			context->CSSetShader(csData.Shader.Get(), nullptr, 0);
			break;
		}
		
		case Geometry:
		{
			const Shader::GeometryShaderData& gsData = shader.As<Shader::Type::Geometry>();
			context->GSSetShader(gsData.Shader.Get(), nullptr, 0);
			break;
		}
		
		case Domain:
		{
			const Shader::DomainShaderData& dsData = shader.As<Shader::Type::Domain>();
			context->DSSetShader(dsData.Shader.Get(), nullptr, 0);
			break;
		}
		
		case Hull:
		{
			const Shader::HullShaderData& hsData = shader.As<Shader::Type::Hull>();
			context->HSSetShader(hsData.Shader.Get(), nullptr, 0);
			break;
		}
		}
	}

	void Renderer::SetConstantBuffers(u32 startSlot, const Shader::Type shaderType, std::span<DX11::IBuffer* const> buffers) const
	{
		DX11::IDeviceContext* const context = m_device->GetContext();

		switch (shaderType)
		{
			using enum Shader::Type;

		case Vertex:
		{
			context->VSSetConstantBuffers(startSlot, static_cast<u32>(buffers.size()), buffers.data());
			break;
		}

		case Pixel:
		{
			context->PSSetConstantBuffers(startSlot, static_cast<u32>(buffers.size()), buffers.data());
			break;
		}

		case Compute:
		{
			context->CSSetConstantBuffers(startSlot, static_cast<u32>(buffers.size()), buffers.data());
			break;
		}

		case Geometry:
		{
			context->GSSetConstantBuffers(startSlot, static_cast<u32>(buffers.size()), buffers.data());
			break;
		}

		case Domain:
		{
			context->DSSetConstantBuffers(startSlot, static_cast<u32>(buffers.size()), buffers.data());
			break;
		}

		case Hull:
		{
			context->HSSetConstantBuffers(startSlot, static_cast<u32>(buffers.size()), buffers.data());
			break;
		}
		}
	}

	void Renderer::CreateDevice(const Core::Device::DeviceDesc& deviceDesc)
	{
		if (auto result = Core::Device::Create(deviceDesc); !result)
		{
			Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("{} (Error Code: {:#x})", result.error().Message, result.error().ErrorCode).Throw();
			return;
		}
		else
		{
			m_device = std::move(result.value());
		}

		Internal::LogAdapterInfo(m_device->GetAdapterInfo());
		Log::Info("Created DX11 Device");
	}

	void Renderer::CreateSwapChain(const Core::SwapChain::SwapChainDesc& swapChainDesc)
	{
		if (auto result = Core::SwapChain::Create(*m_device, swapChainDesc); !result)
		{
			Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("{} (Error Code: {:#x})", result.error().Message, result.error().ErrorCode).Throw();
			return;
		}
		else
		{
			m_swapChain = std::move(result.value());
			Log::Info("Created DX11 Swap Chain");
		}
	}
}
