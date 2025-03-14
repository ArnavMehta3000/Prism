#pragma once
#include "Application/Scene.h"
#include "Graphics/Resources/Buffers/ConstantBuffer.h"
#include "Graphics/Resources/Shaders/Shader.h"

namespace Prism
{
	class SimpleModelScene : public Scene
	{
		static constexpr Elos::StringView AssetPath = PRISM_ASSETS_PATH "/DamagedHelmet.gltf";

	public:
		SimpleModelScene(Elos::Timer* appTimer, Elos::Window* appWindow, AppEvents& appEvents, Gfx::Renderer* renderer);

	private:
		void OnInit() override;
		void OnTick(const Elos::Timer::TimeInfo& timeInfo) override;
		void Render() override;
		void RenderUI() override;
		void OnShutdown() override;
		void LoadModel();
		void LoadShaders();
		void LoadBuffers();

	private:
		std::shared_ptr<Prism::Gfx::Model>        m_model;
		std::shared_ptr<Gfx::ConstantBuffer<WVP>> m_wvpCBuffer;
		std::shared_ptr<Gfx::Shader>              m_shaderVS;
		std::shared_ptr<Gfx::Shader>              m_shaderPS;
	};
}