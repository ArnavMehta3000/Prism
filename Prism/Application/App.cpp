module;
#include "Utils/Log.h"
#include "Application/Globals.h"
#include <Elos/Window/Utils/WindowExtensions.h>
#include <Elos/Common/Assert.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <Windows.h>

module App;
import Scene;
import SimpleModelScene;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Prism
{
	static WNDPROC g_originalWndProc = nullptr;

	static LRESULT ImGuiWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		return ::CallWindowProc(g_originalWndProc, hWnd, msg, wParam, lParam);
	}

	void App::Run()
	{		
		Log::Info("Starting App");
		Init();

		while (m_window->IsOpen())
		{
			ProcessWindowEvents();
			
			m_timer.Tick([this](const Elos::Timer::TimeInfo& timeInfo)
			{
				this->Tick(timeInfo);
			});

			Render();
		}
		
		Log::Info("Stopping App");
		Shutdown();
	}

	void App::Init()
	{
		Elos::ScopedTimer initTimer([](auto timeInfo) { Log::Info("Initialized app in {}s", timeInfo.TotalTime); });

		CreateMainWindow();
		CreateRenderer();
		InitImGui();
		CreateScene();
	}

	void App::Shutdown()
	{
		Elos::ScopedTimer initTimer([](auto timeInfo) { Log::Info("Shutdown app in {}s", timeInfo.TotalTime); });
		
		ShutdownImGui();

		if (m_scene) LIKELY
		{
			m_scene->OnShutdown();
		}

		m_renderer.reset();
		m_window.reset();
	}

	void App::Tick(const Elos::Timer::TimeInfo& timeInfo)
	{
		if (m_scene) LIKELY
		{
			m_scene->OnTick(timeInfo);
		}
	}

	void App::Render()
	{
		if (m_scene) LIKELY
		{
			m_scene->Render();
		}

		if (m_isSolidRenderState) LIKELY
		{
			m_renderer->SetSolidRenderState();
		}
		else
		{
			m_renderer->SetWireframeRenderState();
		}

		m_renderer->BeginEvent(L"ImGui");
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		
		RenderUI();
		if (m_scene) LIKELY
		{
			m_scene->RenderUI();
		}

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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

	void App::InitImGui()
	{
		// Route window procedure
		Elos::ScopedTimer initTimer([](auto timeInfo) { Log::Info("Initialized ImGui in {}s", timeInfo.TotalTime); });
		g_originalWndProc = (WNDPROC)::SetWindowLongPtr(m_window->GetHandle(), GWLP_WNDPROC, (LONG_PTR)&ImGuiWndProc);

		IMGUI_CHECKVERSION();
		MAYBE_UNUSED ImGuiContext* context = ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

#ifdef PRISM_BUILD_DEBUG
		Elos::ASSERT_NOT_NULL(m_renderer.get()).Throw();
#endif

		Elos::ASSERT(ImGui_ImplWin32_Init(m_window->GetHandle())).Msg("Failed to initialize ImGui - Win32").Throw();
		Elos::ASSERT(m_renderer->InitImGui()).Msg("Failed to initialize ImGui - DX11").Throw();
	}

	void App::ShutdownImGui()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void App::RenderUI()
	{
		RenderMainMenuBar();
	}

	void App::RenderMainMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Windows"))
			{
				if (ImGui::BeginMenu("Camera"))
				{
					ImGui::MenuItem("Camera Controls", nullptr, &Globals::g_isCameraControlsWindowOpen);
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::BeginMenu("Camera"))
				{
					ImGui::MenuItem("Camera Debug", nullptr, &Globals::g_isCameraDebugOverlayOpen);
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void App::ProcessWindowEvents()
	{
		const auto OnWindowClosedEvent = [this](const Elos::Event::Closed&)
		{
			m_window->Close();
		};

		const auto OnWindowKeyPressed = [this](const Elos::Event::KeyPressed& e)
		{
			if (e.Key == Elos::KeyCode::Escape)
			{
				Log::Info("Escape pressed...Closing window");
				m_window->Close();
			}

			m_appEvents.OnKeyPressed.Emit(e);

			if (e.Key == Elos::KeyCode::W)  // TODO: Manage using ImGui
			{
				m_isSolidRenderState = !m_isSolidRenderState;
			}
		};

		const auto OnWindowKeyReleased = [this](const Elos::Event::KeyReleased& e)
		{
			m_appEvents.OnKeyReleased.Emit(e);
		};

		const auto OnWindowResizedEvent = [this](const Elos::Event::Resized& e)
		{
			Log::Info("Window resized: {0}x{1}", e.Size.Width, e.Size.Height);

			if (m_renderer)
			{
				m_renderer->Resize(e.Size.Width, e.Size.Height);
			}

			m_appEvents.OnResized.Emit(e);
		};

		const auto OnWindowMouseMoveRaw = [this](const Elos::Event::MouseMovedRaw& e)
		{
			m_appEvents.OnMouseMovedRaw.Emit(e);
		};

		const auto OnWindowMousePressed = [this](const Elos::Event::MouseButtonPressed& e)
		{
			m_appEvents.OnMouseButtonPressed.Emit(e);
		};

		const auto OnWindowMouseReleased = [this](const Elos::Event::MouseButtonReleased& e)
		{
			m_appEvents.OnMouseButtonReleased.Emit(e);
		};

		const auto OnWindowMouseWheel = [this](const Elos::Event::MouseWheelScrolled& e)
		{
			m_appEvents.OnMouseWheelScrolled.Emit(e);
		};


		m_window->HandleEvents(
			OnWindowClosedEvent,
			OnWindowResizedEvent,
			OnWindowKeyPressed,
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
			.SyncInterval = 1,  // No vsync
			.Format       = DXGI_FORMAT_R8G8B8A8_UNORM,
			.AllowTearing = true,
			.Fullscreen   = false
		};

		m_renderer = std::make_unique<Gfx::Renderer>(*m_window, deviceDesc, swapChainDesc, DXGI_FORMAT_R32_TYPELESS);
	}

	void App::CreateScene()
	{
		Elos::ScopedTimer initTimer([](auto timeInfo) { Log::Info("Initialized scene in {}s", timeInfo.TotalTime); });
		
		m_scene = std::make_unique<SimpleModelScene>(
			static_cast<Elos::Timer*>(&m_timer),
			static_cast<Elos::Window*>(m_window.get()),
			m_appEvents,
			static_cast<Gfx::Renderer*>(m_renderer.get())
		);
		m_scene->OnInit();
	}
}
