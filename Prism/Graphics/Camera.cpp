#include "Camera.h"
#include <algorithm>

namespace Prism::Gfx
{
	Camera::Camera(const CameraDesc& desc)
		: m_position(desc.Position)
		, m_fovY(desc.VerticalFOV)
		, m_aspectRatio(desc.AspectRatio)
		, m_nearPlane(desc.NearPlane)
		, m_farPlane(desc.FarPlane)
		, m_orthoWidth(desc.OrthoWidth)
		, m_orthoHeight(desc.OrthoHeight)
		, m_projectionBlend(desc.Projection == ProjectionType::Orthographic ? 1.0f : 0.0f)
		, m_projType(desc.Projection)
	{
		m_forward = desc.LookAt - m_position;
		if (DirectX::XMVector3IsNaN(m_forward) || m_forward.LengthSquared() < kEpsilon)
		{
			m_forward = Vector3::Forward;
		}
		else
		{
			m_forward.Normalize();
		}

		m_up = desc.UpVector;
		if (DirectX::XMVector3IsNaN(m_up) || m_up.LengthSquared() < kEpsilon)
		{
			m_up = Vector3::Up;
		}
		else
		{
			m_up.Normalize();
		}

		m_right = m_forward.Cross(m_up);
		if (DirectX::XMVector3IsNaN(m_right) || m_right.LengthSquared() < kEpsilon)
		{
			m_right = Vector3::Right;
		}
		else
		{
			m_right.Normalize();
		}

		m_up = m_right.Cross(m_forward);
		if (DirectX::XMVector3IsNaN(m_up) || m_up.LengthSquared() < kEpsilon)
		{
			m_up = Vector3::Up;
		}
		else
		{
			m_up.Normalize();
		}

		UpdateViewMatrix();
		UpdateProjectionMatrix();
	}

	void Camera::SetPosition(const Vector3& position)
	{
		m_position = position;
		m_viewDirty = true;
	}

	void Camera::SetLookAt(const Vector3& target)
	{
		if (DirectX::XMVector3IsNaN(target))
		{
			return;
		}

		// Compute new forward direction
		Vector3 newForward = target - m_position;
		if (newForward.LengthSquared() > kEpsilon)
		{
			m_forward = newForward;
			m_forward.Normalize();
		}
		else
		{
			m_forward = Vector3::Forward;
		}

		// Recompute right vector with current up
		m_right = m_forward.Cross(m_up);
		if (m_right.LengthSquared() > kEpsilon)
		{
			m_right.Normalize();
		}
		else
		{
			m_right = Vector3::Right;
		}

		// Recompute up vector for consistency
		m_up = m_right.Cross(m_forward);
		if (m_up.LengthSquared() > kEpsilon)
		{
			m_up.Normalize();
		}
		else
		{
			m_up = Vector3::Up;
		}

		m_viewDirty = true;
	}

	void Camera::SetUpVector(const Vector3& up)
	{
		if (DirectX::XMVector3IsNaN(up))
		{
			return;
		}

		// Use provided up if valid; otherwise, fallback.
		Vector3 newUp = up;
		if (newUp.LengthSquared() > kEpsilon)
		{
			newUp.Normalize();
		}
		else
		{
			newUp = Vector3::Up;
		}

		m_up = newUp;
		m_right = m_forward.Cross(m_up);
		if (m_right.LengthSquared() > kEpsilon)
		{
			m_right.Normalize();
		}
		else
		{
			m_right = Vector3::Right;
		}

		m_up = m_right.Cross(m_forward);
		if (m_up.LengthSquared() > kEpsilon)
		{
			m_up.Normalize();
		}
		else
		{
			m_up = Vector3::Up;
		}

		m_viewDirty = true;
	}

	void Camera::SetOrientation(const Quaternion& orientation)
	{
		Matrix rotationMatrix = Matrix::CreateFromQuaternion(orientation);
		m_forward             = Vector3::Transform(Vector3::Forward, rotationMatrix);
		m_up                  = Vector3::Transform(Vector3::Up, rotationMatrix);
		m_right               = Vector3::Transform(Vector3::Right, rotationMatrix);

		m_viewDirty = true;
	}

	void Camera::SetPerspective(const f32 fovY, const f32 aspect, const f32 nearPlane, const f32 farPlane)
	{
		m_fovY        = fovY;
		m_aspectRatio = aspect;
		m_nearPlane   = nearPlane;
		m_farPlane    = farPlane;
		m_projDirty   = true;
	}

	void Camera::SetOrthographic(const f32 width, const f32 height, const f32 nearPlane, const f32 farPlane)
	{
		m_orthoWidth  = width;
		m_orthoHeight = height;
		m_nearPlane   = nearPlane;
		m_farPlane    = farPlane;
		m_projDirty   = true;
	}

	void Camera::SetAspectRatio(const f32 aspect)
	{
		if (aspect > 0.0f)
		{
			m_aspectRatio = aspect;
			m_orthoWidth = m_orthoHeight * aspect; // Assuming orthographic height remains constant
			m_projDirty = true;
		}
	}

	void Camera::SetProjectionType(ProjectionType type, f32 blendFactor)
	{
		m_projType = type;
		m_projectionBlend = type == ProjectionType::Orthographic ?
			std::min(1.0f, std::max(0.0f, blendFactor)) :
			std::min(1.0f, std::max(0.0f, 1.0f - blendFactor));
		m_projDirty = true;
	}

	void Camera::Translate(const Vector3& delta)
	{
		m_position += delta;
		m_viewDirty = true;
	}

	void Camera::Rotate(const Quaternion& rotation)
	{
		Matrix rotationMatrix = Matrix::CreateFromQuaternion(rotation);

		m_forward = Vector3::Transform(m_forward, rotationMatrix);
		m_up      = Vector3::Transform(m_up, rotationMatrix);
		m_right   = Vector3::Transform(m_right, rotationMatrix);

		m_viewDirty = true;
	}

	void Camera::RotateAround(const Vector3& point, const Vector3& axis, f32 angle)
	{
		// Create rotation matrix around the given point
		Matrix rotation = Matrix::CreateTranslation(-point) *
			Matrix::CreateFromAxisAngle(axis, angle) *
			Matrix::CreateTranslation(point);

		// Transform position
		m_position = Vector3::Transform(m_position, rotation);

		// Rotate orientation vectors
		Matrix rotationOnly = Matrix::CreateFromAxisAngle(axis, angle);
		m_forward           = Vector3::Transform(m_forward, rotationOnly);
		m_up                = Vector3::Transform(m_up, rotationOnly);
		m_right             = Vector3::Transform(m_right, rotationOnly);

		m_viewDirty = true;
	}

	void Camera::Update()
	{
		if (m_viewDirty)
		{
			UpdateViewMatrix();
			m_viewDirty = false;
		}

		if (m_projDirty)
		{
			UpdateProjectionMatrix();
			m_projDirty = false;
		}
	}

	void Camera::Resize(const u32 width, const u32 height)
	{
		if (height > 0)
		{
			SetAspectRatio(static_cast<f32>(width) / static_cast<f32>(height));
			m_projDirty = true;
		}
	}

	void Camera::SetZoomLevel(f32 newZoomLevel)
	{
		m_zoomLevel = std::clamp(newZoomLevel, MinZoomLevel, MaxZoomLevel);

		// Calculate FOV and ortho height based on zoom level
		// Inverse relationship: higher zoom level = smaller FOV / smaller ortho height
		f32 zoomFactor = 1.0f / m_zoomLevel;

		m_fovY = std::clamp(DefaultFOV * zoomFactor, MinFOV, MaxFOV);

		m_orthoHeight = std::clamp(DefaultOrthoHeight * zoomFactor, MinOrthoHeight, MaxOrthoHeight);
		m_orthoWidth = m_orthoHeight * m_aspectRatio;

		m_projDirty = true;
	}

	void Camera::ZoomBy(f32 zoomDelta)
	{
		// Exponential zoom for more natural feel
		// Positive delta zooms in, negative delta zooms out
		f32 newZoomLevel;

		if (zoomDelta > 0.0f)
		{
			// Zoom in: multiply by zoom factor for smoother zoom at higher levels
			newZoomLevel = m_zoomLevel * (1.0f + zoomDelta * 0.1f);
		}
		else
		{
			// Zoom out: divide by zoom factor for smoother zoom at lower levels
			newZoomLevel = m_zoomLevel / (1.0f - zoomDelta * 0.1f);
		}

		SetZoomLevel(newZoomLevel);
	}
	
	void Camera::UpdateViewMatrix()
	{
		m_viewMatrix = Matrix::CreateLookAt(m_position, m_forward + m_position, m_up);
	}
	
	void Camera::UpdateProjectionMatrix()
	{
		if (m_projectionBlend <= 0.0f)
		{
			m_projMatrix = CalculatePerspectiveMatrix();  // Pure perspective
		}
		else if (m_projectionBlend >= 1.0f)
		{
			m_projMatrix = CalculateOrthographicMatrix(); // Pure orthographic
		}
		else
		{
			// Blend between perspective and orthographic
			Matrix perspMatrix = CalculatePerspectiveMatrix();
			Matrix orthoMatrix = CalculateOrthographicMatrix();
			m_projMatrix = Matrix::Lerp(perspMatrix, orthoMatrix, m_projectionBlend);
		}

	}
	
	Matrix Camera::CalculatePerspectiveMatrix() const
	{
		const f32 fovRadians = DirectX::XMConvertToRadians(m_fovY);
		return Matrix::CreatePerspectiveFieldOfView(fovRadians, m_aspectRatio, m_nearPlane, m_farPlane);
	}
	
	Matrix Camera::CalculateOrthographicMatrix() const
	{
		return Matrix::CreateOrthographic(m_orthoWidth, m_orthoHeight, m_nearPlane, m_farPlane);
	}
}
