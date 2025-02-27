#pragma once
#include "Math/Math.h"
#include <Elos/Common/FunctionMacros.h>

namespace Prism::Gfx
{
	class Camera
	{
	public:
		constexpr static f32 MinFOV             = 2.0f;
		constexpr static f32 MaxFOV             = 120.0f;
		constexpr static f32 MinOrthoHeight     = 1.0f;
		constexpr static f32 MaxOrthoHeight     = 100.0f;
		constexpr static f32 MinZoomLevel       = 0.1f;   // 10x zoom out
		constexpr static f32 MaxZoomLevel       = 15.0f;  // 15x zoom in
		constexpr static f32 DefaultFOV         = 60.0f;
		constexpr static f32 DefaultOrthoHeight = 20.0f;

		enum class ProjectionType { Perspective, Orthographic };

		struct CameraDesc
		{
			f32 VerticalFOV           = 45.0f;
			f32 AspectRatio           = 16.0f / 9.0f;
			f32 NearPlane             = 0.3f;
			f32 FarPlane              = 1000.0f;
			f32 OrthoWidth            = 20.0f;
			f32 OrthoHeight           = 11.25;
			Vector3 Position          = Vector3(0.0f, 5.0f, 15.0f);
			Vector3 LookAt            = Vector3::Zero;
			Vector3 UpVector          = Vector3::Up;
			ProjectionType Projection = ProjectionType::Perspective;
		};

	public:
		Camera(const CameraDesc& desc = CameraDesc{});
		~Camera() = default;

		void SetPosition(const Vector3& position);
		void SetLookAt(const Vector3& target);
		void SetUpVector(const Vector3& up);
		void SetOrientation(const Quaternion& orientation);
		void SetPerspective(const f32 fovY, const f32 aspect, const f32 nearPlane, const f32 farPlane);
		void SetOrthographic(const f32 width, const f32 height, const f32 nearPlane, const f32 farPlane);
		void SetAspectRatio(const f32 aspect);
		void SetProjectionType(ProjectionType type, f32 blendFactor = 1.0f);
		void Translate(const Vector3& delta);
		void Rotate(const Quaternion& rotation);
		void RotateAround(const Vector3& point, const Vector3& axis, f32 angle);
		void Update();
		void Resize(const u32 width, const u32 height);
		void SetZoomLevel(f32 newZoomLevel);
		void ZoomBy(f32 zoomDelta);

		NODISCARD Matrix GetViewMatrix() const { return m_viewMatrix; }
		NODISCARD Matrix GetProjectionMatrix() const { return m_projMatrix; }
		NODISCARD Matrix GetViewProjectionMatrix() const { return m_viewMatrix * m_projMatrix; }
		NODISCARD Vector3 GetPosition() const { return m_position; }
		NODISCARD Vector3 GetForwardVector() const { return m_forward; }
		NODISCARD Vector3 GetRightVector() const { return m_right; }
		NODISCARD Vector3 GetUpVector() const { return m_up; }
		NODISCARD f32 GetNearPlane() const { return m_nearPlane; }
		NODISCARD f32 GetFarPlane() const { return m_farPlane; }
		NODISCARD f32 GetFOV() const { return m_fovY; }
		NODISCARD f32 GetAspectRatio() const { return m_aspectRatio; }
		NODISCARD f32 GetOrthoWidth() const { return m_orthoWidth; }
		NODISCARD f32 GetOrthoHeight() const { return m_orthoHeight; }
		NODISCARD f32 GetProjectionBlend() const { return m_projectionBlend; }
		NODISCARD ProjectionType GetProjectionType() const { return m_projType; }
		NODISCARD f32 GetZoomLevel() const { return m_zoomLevel; }

	private:
		void UpdateViewMatrix();
		void UpdateProjectionMatrix();
		Matrix CalculatePerspectiveMatrix() const;
		Matrix CalculateOrthographicMatrix() const;

	private:
		Matrix         m_viewMatrix;
		Matrix         m_projMatrix;
		Vector3        m_position;
		Vector3        m_forward;
		Vector3        m_up;
		Vector3        m_right;
		f32            m_fovY;
		f32            m_aspectRatio;
		f32            m_nearPlane;
		f32            m_farPlane;
		f32            m_orthoWidth;
		f32            m_orthoHeight;
		f32            m_projectionBlend;  // 0.0 = perspective, 1.0 = orthographic
		f32            m_zoomLevel = 1.0f;
		ProjectionType m_projType;
		bool           m_viewDirty = true;
		bool           m_projDirty = true;
	};
}
