#include "Renderer.h"
#include "Utils/Log.h"
#include "Graphics/Utils/ResourceFactory.h"
#include "Graphics/Utils/DebugName.h"
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

	Renderer::Renderer(Elos::Window& window, const Core::Device::DeviceDesc& deviceDesc,
		const Core::SwapChain::SwapChainDesc& swapChainDesc, const DXGI_FORMAT depthFormat)
		: m_depthStencilFormat(depthFormat)
		, m_window(window)
	{
		CreateDevice(deviceDesc);
		CreateSwapChain(swapChainDesc);
		CreateDefaultStates();

		if (auto result = CreateDepthStencilBuffer(); !result)
		{
			Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("{} (Error Code: {:#x})", result.error().Message, result.error().ErrorCode).Throw();
		}

		m_resourceFactory = std::make_unique<ResourceFactory>(m_device.get());

		Log::Info("Created Renderer");
	}

	Renderer::~Renderer()
	{
		Log::Info("Shutting down Renderer");
		m_depthStencilBuffer.Reset();
		m_depthStencilView.Reset();
		m_defaultDepthStencilState.Reset();
		m_wireframeRasterizerState.Reset();
		m_solidRasterizerState.Reset();
		m_resourceFactory.reset();
		m_swapChain.reset();
		m_device.reset();
	}

	bool Renderer::IsGraphicsDebuggerAttached() const
	{
		if (auto perf = m_device->GetAnnotation())
		{
			return perf->GetStatus();
		}

		return false;
	}

	void Renderer::BeginEvent(const Elos::WString& eventName) const
	{
		if (auto perf = m_device->GetAnnotation())
		{
			perf->BeginEvent(eventName.c_str());
		}
	}

	void Renderer::EndEvent() const
	{
		if (auto perf = m_device->GetAnnotation())
		{
			perf->EndEvent();
		}
	}

	void Renderer::SetMarker(const Elos::WString& markerName) const
	{
		if (auto perf = m_device->GetAnnotation())
		{
			perf->SetMarker(markerName.c_str());
		}
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

	void Renderer::ClearDepthStencilBuffer(const u32 flag, const f32 depth, const u8 stencil) const
	{
		m_device->GetContext()->ClearDepthStencilView(m_depthStencilView.Get(), flag, depth, stencil);
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

	void Renderer::Resize(const u32 width, const u32 height)
	{
		if (width == 0 || height == 0)
		{
			return;  // We cannot resize
		}

		m_depthStencilView.Reset();
		m_depthStencilBuffer.Reset();

		if (m_swapChain)
		{
			// Only check for resize failure in debug builds
#if PRISM_BUILD_DEBUG
			if (auto result = m_swapChain->Resize(width, height); !result)
			{
				Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("Failed to resize DXGI Swap Chain! (Error Code: {:#x})", result.error().ErrorCode).Throw();
			}

			if (auto result = CreateDepthStencilBuffer(); !result)
			{
				Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("Failed to resize Depth Stencil Buffer! {} (Error Code: {:#x})", result.error().Message, result.error().ErrorCode).Throw();
			}
#else
			std::ignore = m_swapChain->Resize(width, height);
			std::ignore = CreateDepthStencilBuffer();
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
		m_device->GetContext()->OMSetRenderTargets(1, &backBufferRTV, m_depthStencilView.Get());
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

	void Renderer::SetDepthStencilState(DX11::IDepthStencilState* state, u32 stencilRef) const
	{
		m_device->GetContext()->OMSetDepthStencilState(state, stencilRef);
	}

	void Renderer::SetRasterizerState(DX11::IRasterizerState* state) const
	{
		m_device->GetContext()->RSSetState(state);
	}

	void Renderer::SetSolidRenderState() const
	{
		SetRasterizerState(m_solidRasterizerState.Get());
		SetDepthStencilState(m_defaultDepthStencilState.Get(), 0);
	}

	void Renderer::SetWireframeRenderState() const
	{
		SetRasterizerState(m_wireframeRasterizerState.Get());
		SetDepthStencilState(m_defaultDepthStencilState.Get(), 0);
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

	void Renderer::CreateDefaultStates()
	{
		// Create default depth stencil state
		D3D11_DEPTH_STENCIL_DESC dsDesc{};
		dsDesc.DepthEnable                  = true;
		dsDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc                    = D3D11_COMPARISON_LESS;
		dsDesc.StencilEnable                = true;
		dsDesc.StencilReadMask              = 0xFF;
		dsDesc.StencilWriteMask             = 0xFF;
		dsDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		dsDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
		dsDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_DECR;
		dsDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

		HRESULT hr = m_device->GetDevice()->CreateDepthStencilState(&dsDesc, &m_defaultDepthStencilState);
		if (FAILED(hr))
		{
			Log::Error("Failed to create default depth stencil state. Error code: {:#x}", hr);
		}

		// Create solid rasterizer state
		CD3D11_RASTERIZER_DESC1 solidRastDesc = CD3D11_RASTERIZER_DESC1(CD3D11_DEFAULT());
		solidRastDesc.FillMode = D3D11_FILL_SOLID;
		solidRastDesc.CullMode = D3D11_CULL_BACK;
		hr = m_device->GetDevice()->CreateRasterizerState1(&solidRastDesc, &m_solidRasterizerState);
		if (FAILED(hr))
		{
			Log::Error("Failed to create solid rasterizer state. Error code: {:#x}", hr);
		}

		// Create wireframe rasterizer state
		CD3D11_RASTERIZER_DESC1 wireframeRastDesc = solidRastDesc;
		wireframeRastDesc.FillMode = D3D11_FILL_WIREFRAME;

		hr = m_device->GetDevice()->CreateRasterizerState1(&wireframeRastDesc, &m_wireframeRasterizerState);
		if (FAILED(hr))
		{
			Log::Error("Failed to create wireframe rasterizer state. Error code: {:#x}", hr);
		}
	}

	std::expected<void, Core::SwapChain::SwapChainError> Renderer::CreateDepthStencilBuffer()
	{
		const auto& swapChainDesc = m_swapChain->GetDesc();

		m_depthStencilBuffer.Reset();
		m_depthStencilView.Reset();

		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width              = swapChainDesc.Width;
		depthStencilDesc.Height             = swapChainDesc.Height;
		depthStencilDesc.MipLevels          = 1;
		depthStencilDesc.ArraySize          = 1;
		depthStencilDesc.Format             = m_depthStencilFormat;
		depthStencilDesc.SampleDesc.Count   = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage              = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags     = 0;
		depthStencilDesc.MiscFlags          = 0;

		HRESULT hr = S_OK;
		{
			ComPtr<ID3D11Texture2D> depthTexture;
			hr = m_device->GetDevice()->CreateTexture2D(&depthStencilDesc, nullptr, &depthTexture);
			if (FAILED(hr))
			{
				return std::unexpected(Core::SwapChain::SwapChainError
				{
					.Type = Core::SwapChain::SwapChainError::Type::CreateRTVFailed,
					.ErrorCode = hr,
					.Message = "Failed to create depth stencil buffer"
				});
			}

			if (FAILED(depthTexture.As(&m_depthStencilBuffer)))
			{
				return std::unexpected(Core::SwapChain::SwapChainError
				{
					.Type = Core::SwapChain::SwapChainError::Type::CreateRTVFailed,
					.ErrorCode = hr,
					.Message = "Failed to create depth stencil buffer"
				});
			}
			SetDebugObjectName(m_depthStencilBuffer, "DX11DepthStencilBuffer");
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format             = depthStencilDesc.Format == DXGI_FORMAT_R32_TYPELESS ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;

		hr = m_device->GetDevice()->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, &m_depthStencilView);
		if (FAILED(hr))
		{
			return std::unexpected(Core::SwapChain::SwapChainError
			{
				.Type      = Core::SwapChain::SwapChainError::Type::CreateRTVFailed,
				.ErrorCode = hr,
				.Message   = "Failed to create depth stencil view"
			});
		}

		SetDebugObjectName(m_depthStencilView, "DX11DepthStencilView");

		return {};
	}
}
