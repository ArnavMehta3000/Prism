#pragma once
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Prism
{
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
}

namespace Prism::DX11
{
	using IDevice            = ID3D11Device5;
	using IDeviceContext     = ID3D11DeviceContext4;
	using ICommandList       = ID3D11CommandList;
	using IRenderTarget      = ID3D11RenderTargetView1;
	using ITexture2D         = ID3D11Texture2D1;
	using IDepthStencil      = ID3D11DepthStencilView;
	using IShaderResource    = ID3D11ShaderResourceView1;
	using IVertexShader      = ID3D11VertexShader;
	using IPixelShader       = ID3D11PixelShader;
	using IGeometryShader    = ID3D11GeometryShader;
	using IComputeShader     = ID3D11ComputeShader;
	using IHullShader        = ID3D11HullShader;
	using IDomainShader      = ID3D11DomainShader;
	using IUnorderedAccess   = ID3D11UnorderedAccessView1;
	using IInputLayout       = ID3D11InputLayout;
	using ISamplerState      = ID3D11SamplerState;
	using IBlendState        = ID3D11BlendState;
	using IDepthStencilState = ID3D11DepthStencilState;
	using IRasterizerState   = ID3D11RasterizerState1;
	using IBuffer            = ID3D11Buffer;
	using ISwapChain         = IDXGISwapChain4;
	using IBlob              = ID3DBlob;
	using IFactory           = IDXGIFactory6;
	using IAdapter           = IDXGIAdapter4;
}
