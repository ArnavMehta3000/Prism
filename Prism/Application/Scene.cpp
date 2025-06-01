#include "Scene.h"
#include "Application/Globals.h"
#include <Elos/Window/Window.h>
#include <Elos/Common/Assert.h>
#include <imgui.h>

namespace Prism
{
	Scene::Scene(Elos::Timer* appTimer, Elos::Window* appWindow, AppEvents& appEvents, Gfx::Renderer* renderer)
		: m_appTimer(appTimer)
		, m_appWindow(appWindow)
		, m_renderer(renderer)
	{
#ifdef PRISM_BUILD_DEBUG
		Elos::ASSERT(m_appTimer && m_appWindow && m_renderer)
			.Msg("Failed to create scene. One or more input pointers are nullptr")
			.Throw();
#endif

		// Create camera
		Gfx::Camera::CameraDesc cameraDesc;
		auto [width, height]   = m_appWindow->GetSize();
		
		cameraDesc.AspectRatio = static_cast<f32>(width) / static_cast<f32>(height);
		cameraDesc.Position    = Vector3(0.0f, 0.0f, 15.0f);
		cameraDesc.LookAt      = Vector3::Zero;
		m_camera               = std::make_unique<Gfx::Camera>(cameraDesc);
		m_cameraController     = std::make_unique<CameraController>(m_camera.get());

		// Bind scene events
		BindToAppEvents(appEvents);
	}

	Scene::~Scene()
	{
		m_connections.DisconnectAll();
		m_camera.reset();
	}

	void Scene::OnTick(const Elos::Timer::TimeInfo& timeInfo)
	{
		if (m_cameraController) LIKELY
		{
			m_cameraController->Update(timeInfo);
		}
	}

	void Scene::RenderUI()
	{
		RenderCameraDebugOverlay();
		RenderCameraControlsWindow();
	}

	void Scene::OnResize(const Elos::Event::Resized& e)
	{
		m_camera->Resize(e.Size.Width, e.Size.Height);
	}

#pragma region Input
	void Scene::OnKeyPressed(const Elos::Event::KeyPressed& e)
	{
		if (m_cameraController) LIKELY
		{
			m_cameraController->OnKeyPressed(e);
		}
	}

	void Scene::OnKeyReleased(const Elos::Event::KeyReleased& e)
	{
		if (m_cameraController) LIKELY
		{
			m_cameraController->OnKeyReleased(e);
		}
	}

	void Scene::OnMouseMovedRaw(const Elos::Event::MouseMovedRaw& e)
	{
		if (m_cameraController) LIKELY
		{
			m_cameraController->OnMouseMovedRaw(e);
		}
	}

	void Scene::OnMouseButtonPressed(const Elos::Event::MouseButtonPressed& e)
	{
		if (m_cameraController) LIKELY
		{
			m_cameraController->OnMouseButtonPressed(e);
		}
	}

	void Scene::OnMouseButtonReleased(const Elos::Event::MouseButtonReleased& e)
	{
		if (m_cameraController) LIKELY
		{
			m_cameraController->OnMouseButtonReleased(e);
		}
	}

	void Scene::OnMouseWheelScrolled(const Elos::Event::MouseWheelScrolled& e)
	{
		if (m_cameraController) LIKELY
		{
			m_cameraController->OnMouseWheelScrolled(e);
		}
	}
#pragma endregion

#pragma region ImGui
	void Scene::RenderCameraDebugOverlay()
	{
		if (!Globals::g_isCameraDebugOverlayOpen)
		{
			return;
		}

		static int location = 1;
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_AlwaysAutoResize
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoFocusOnAppearing
			| ImGuiWindowFlags_NoNav;

		if (location >= 0)
		{
			ImVec2 windowPos, windowPosPivot;

			constexpr float PAD = 10.0f;
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 workPos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
			ImVec2 workSize = viewport->WorkSize;

			windowPos.x      = (location & 1) ? (workPos.x + workSize.x - PAD) : (workPos.x + PAD);
			windowPos.y      = (location & 2) ? (workPos.y + workSize.y - PAD) : (workPos.y + PAD);
			windowPosPivot.x = (location & 1) ? 1.0f : 0.0f;
			windowPosPivot.y = (location & 2) ? 1.0f : 0.0f;

			ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
			ImGui::SetNextWindowViewport(viewport->ID);
			windowFlags |= ImGuiWindowFlags_NoMove;
		}
		else if (location == -2)
		{
			// Center window
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			windowFlags |= ImGuiWindowFlags_NoMove;
		}

		ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

		if (ImGui::Begin("Camera Debug##Window", nullptr, windowFlags))
		{
			const Vector3& currentVelocity = m_cameraController->GetCurrentVelocity();
			const Vector3 position         = m_camera->GetPosition();
			const Vector3 forward          = m_camera->GetForwardVector();
			const f32 zoom                 = m_camera->GetZoomLevel();
			const f32 fov                  = m_camera->GetFOV();
			const f32 aspectRatio          = m_camera->GetAspectRatio();

			ImGui::Text("Camera Debug Info");
			ImGui::Separator();
			ImGui::Text("Camera Position: %.2f, %.2f, %.2f", position.x, position.y, position.z);
			ImGui::Text("Look Direction: %.2f, %.2f, %.2f", forward.x, forward.y, forward.z);
			ImGui::Separator();
			ImGui::Text("Current Velocity: %.2f, %.2f, %.2f", currentVelocity.x, currentVelocity.y, currentVelocity.z);
			ImGui::Separator();
			ImGui::Text("Zoom Level: %.2f", zoom);
			ImGui::Text("FOV: %.2f", fov);
			ImGui::Text("Aspect Ratio: %.2f", aspectRatio);

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Custom", NULL, location == -1)) location = -1;
				if (ImGui::MenuItem("Center", NULL, location == -2)) location = -2;
				if (ImGui::MenuItem("Top-left", NULL, location == 0)) location = 0;
				if (ImGui::MenuItem("Top-right", NULL, location == 1)) location = 1;
				if (ImGui::MenuItem("Bottom-left", NULL, location == 2)) location = 2;
				if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;

				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	void Scene::RenderCameraControlsWindow()
	{
		if (!Globals::g_isCameraControlsWindowOpen || !m_cameraController)
		{
			return;
		}

		if (ImGui::Begin("Camera Controls##Window", nullptr))
		{
			CameraController::Settings& settings = m_cameraController->GetSettings();

			if (ImGui::CollapsingHeader("Movement"))
			{
				ImGui::DragFloat("Move Speed", &settings.MoveSpeed, 0.1f);
				ImGui::DragFloat("Rotation Speed", &settings.RotationSpeed, 0.1f);
				ImGui::DragFloat("Zoom Sensitivity", &settings.ZoomSensitivity, 0.05f);
			}
			if (ImGui::CollapsingHeader("Smoothing"))
			{
				ImGui::DragFloat("Movement Smooth Time", &settings.MovementSmoothTime, 0.01f);
				ImGui::DragFloat("Movement Damping", &settings.MovementDamping, 0.01f);
			}
			if (ImGui::CollapsingHeader("Zoom & Projection"))
			{
				f32 projBlend = m_camera->GetProjectionBlend();
				if (ImGui::SliderFloat("Projection", &projBlend, 0.0f, 1.0f))
				{
					const Gfx::Camera::ProjectionType type = projBlend > 0.5f ?
						Gfx::Camera::ProjectionType::Orthographic :
						Gfx::Camera::ProjectionType::Perspective;
					
					m_camera->SetProjectionType(type, 
						(type == Gfx::Camera::ProjectionType::Orthographic) ?
						projBlend : 1.0f - projBlend);
				}

				f32 zoom = m_camera->GetZoomLevel();
				if (ImGui::SliderFloat("Zoom", &zoom, Gfx::Camera::MinZoomLevel, Gfx::Camera::MaxZoomLevel))
				{
					m_camera->SetZoomLevel(zoom);
				}
			}

			m_cameraController->SetSettings(settings);

			if (ImGui::Button("Reset Camera"))
			{
				m_cameraController->ResetCamera();
				m_camera->SetZoomLevel(1.0f);
			}
		}
		ImGui::End();
	}
#pragma endregion

	
	void Scene::BindToAppEvents(AppEvents& appEvents)
	{
		m_connections.OnResized             = appEvents.OnResized.Connect([this](const auto& e)             { OnResize(e);              });
		m_connections.OnKeyPressed          = appEvents.OnKeyPressed.Connect([this](const auto& e)          { OnKeyPressed(e);          });
		m_connections.OnKeyReleased         = appEvents.OnKeyReleased.Connect([this](const auto& e)         { OnKeyReleased(e);         });
		m_connections.OnMouseButtonPressed  = appEvents.OnMouseButtonPressed.Connect([this](const auto& e)  { OnMouseButtonPressed(e);  });
		m_connections.OnMouseButtonReleased = appEvents.OnMouseButtonReleased.Connect([this](const auto& e) { OnMouseButtonReleased(e); });
		m_connections.OnMouseMovedRaw       = appEvents.OnMouseMovedRaw.Connect([this](const auto& e)       { OnMouseMovedRaw(e);       });
		m_connections.OnMouseWheelScrolled  = appEvents.OnMouseWheelScrolled.Connect([this](const auto& e)  { OnMouseWheelScrolled(e);  });
	}
}
