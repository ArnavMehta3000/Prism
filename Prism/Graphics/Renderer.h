#pragma once
#include "Graphics/DX11Types.h"
#include "Graphics/Core/Device.h"
#include "Graphics/Core/SwapChain.h"
#include "Graphics/Resources/Buffers/ConstantBuffer.h"
#include "Graphics/Resources/Shaders/Shader.h"
#include <span>

namespace Elos
{
	class Window;
}

namespace Prism::Gfx
{
	class Mesh;
	class Shader;
	class Camera;
	class ResourceFactory;
	class IndexBuffer;
	class VertexBuffer;

	class Renderer
	{
	public:
		Renderer(Elos::Window& window, const Core::Device::DeviceDesc& deviceDesc,
			const Core::SwapChain::SwapChainDesc& swapChainDesc, const DXGI_FORMAT depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);
		~Renderer();

		bool InitImGui();

		NODISCARD const ResourceFactory& GetResourceFactory() const { return *m_resourceFactory; }
		NODISCARD bool IsGraphicsDebuggerAttached() const;
		void BeginEvent(const Elos::WString& eventName) const;
		void EndEvent() const;
		void SetMarker(const Elos::WString& markerName) const;

		void ClearState() const;
		void ClearBackBuffer(const f32* clearColor) const;
		void SetViewports(const std::span<D3D11_VIEWPORT> viewports) const;
		void ClearDepthStencilBuffer(const u32 flag, const f32 depth = 1.0f, const u8 stencil = 0) const;
		void SetWindowAsViewport() const;
		void Resize(const u32 width, const u32 height);
		void Present() const;
		void Flush() const;
		void SetBackBufferRenderTarget() const;
		void Draw(const u32 vertexCount, const u32 startIndex) const;
		void DrawAuto() const;
		void DrawIndexed(const u32 indexCount, const u32 startIndexLocation, const i32 baseVertexLocation) const;
		void DrawIndexedInstanced(const u32 indexCountPerInstance, const u32 instanceCount, const u32 startIndexLocation, const i32 baseVertexLocation, const u32 startInstanceLocation) const;
		void DrawInstanced(const u32 vertexCountPerInstance, const u32 instanceCount, const u32 startVertexLocation, const u32 startInstanceLocation) const;
		void SetShader(const Shader& shader) const;
		void SetConstantBuffers(u32 startSlot, const Shader::Type shaderType, const std::span<const Buffer* const> buffers) const;
		void SetDepthStencilState(DX11::IDepthStencilState* state, u32 stencilRef = 0) const;
		void SetRasterizerState(DX11::IRasterizerState* state) const;
		void SetSamplerState(const Shader::Type shaderType, const u32 slot, const std::span<DX11::ISamplerState* const> samplers) const;
		void SetShaderResourceViews(const Shader::Type shaderType, const u32 slot, std::span<ID3D11ShaderResourceView* const> views) const;
		void SetSolidRenderState() const;
		void SetWireframeRenderState() const;
		void SetIndexBuffer(const IndexBuffer& buffer, const DXGI_FORMAT format = DXGI_FORMAT_R32_UINT, const u32 offset = 0) const noexcept;
		void SetVertexBuffers(const u32 startSlot, const std::span<const VertexBuffer* const>& buffers, std::span<const u32> offsets) const noexcept;
		void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology) const noexcept;

		template<typename T>
		std::expected<void, Buffer::BufferError> UpdateConstantBuffer(ConstantBuffer<T>& constantBuffer, const T& data) const { return constantBuffer.Update(m_device->GetContext(), data); }

		NODISCARD inline Core::Device* GetDevice() const noexcept { return m_device.get(); }

	private:
		void CreateDevice(const Core::Device::DeviceDesc& deviceDesc);
		void CreateSwapChain(const Core::SwapChain::SwapChainDesc& swapChainDesc);
		void CreateDefaultStates();
		std::expected<void, Core::SwapChain::SwapChainError> CreateDepthStencilBuffer();

		NODISCARD inline Core::SwapChain* GetSwapChain() const noexcept { return m_swapChain.get(); }

	private:
		DXGI_FORMAT                      m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		Elos::Window&                    m_window;
		std::unique_ptr<ResourceFactory> m_resourceFactory;
		std::unique_ptr<Core::Device>    m_device;
		std::unique_ptr<Core::SwapChain> m_swapChain;
		ComPtr<DX11::IDepthStencilState> m_defaultDepthStencilState;
		ComPtr<DX11::IRasterizerState>   m_solidRasterizerState;
		ComPtr<DX11::IRasterizerState>   m_wireframeRasterizerState;
		ComPtr<DX11::ITexture2D>         m_depthStencilBuffer;
		ComPtr<DX11::IDepthStencil>      m_depthStencilView;
	};
}
