#include "CameraController.h"

namespace Prism
{
	CameraController::CameraController(Gfx::Camera* camera, const Settings& settings)
		: m_camera(camera)
		, m_settings(settings)
		, m_targetPosition(camera->GetPosition())
		, m_targetOrbitDistance(settings.OrbitDistance)
	{
		Vector3 forward = m_camera->GetForwardVector();
		m_yaw = std::atan2f(forward.x, forward.z);
		m_pitch = std::asinf(forward.y);
	}
	
	void CameraController::Update(const Elos::Timer::TimeInfo& timeInfo)
	{
		if (m_settings.FirstPersonMode)
		{
			UpdateFirstPersonCamera(timeInfo.DeltaTime);
		}
		else
		{
			UpdateOrbitCamera(timeInfo.DeltaTime);
		}
	}
	
	void CameraController::OnKeyPressed(const Elos::Event::KeyPressed& e)
	{
		if (e.Key == m_settings.ForwardKey)
		{
			m_movingForward = true;
		}
		else if (e.Key == m_settings.BackwardKey)
		{
			m_movingBackward = true;
		}
		else if (e.Key == m_settings.LeftKey)
		{
			m_movingLeft = true;
		}
		else if (e.Key == m_settings.RightKey)
		{
			m_movingRight = true;
		}
		else if (e.Key == m_settings.UpKey)
		{
			m_movingUp = true;
		}
		else if (e.Key == m_settings.DownKey)
		{
			m_movingDown = true;
		}
		else if (e.Key == m_settings.SpeedBoostKey || e.Control)
		{
			m_speedBoost = true;
		}
		else if (e.Key == m_settings.SpeedSlowKey || e.Shift)
		{
			m_speedSlow = true;
		}
		else if (e.Key == m_settings.OrbitToggleKey)
		{
			SetFirstPersonMode(!m_settings.FirstPersonMode);
		}
	}
	
	void CameraController::OnKeyReleased(const Elos::Event::KeyReleased& e)
	{
		if(e.Key == m_settings.ForwardKey)
		{
			m_movingForward = false;
		}
		else if (e.Key == m_settings.BackwardKey)
		{
			m_movingBackward = false;
		}
		else if (e.Key == m_settings.LeftKey)
		{
			m_movingLeft = false;
		}
		else if (e.Key == m_settings.RightKey)
		{
			m_movingRight = false;
		}
		else if (e.Key == m_settings.UpKey)
		{
			m_movingUp = false;
		}
		else if (e.Key == m_settings.DownKey)
		{
			m_movingDown = false;
		}
		else if (e.Key == m_settings.SpeedBoostKey || e.Control)
		{
			m_speedBoost = false;
		}
		else if (e.Key == m_settings.SpeedSlowKey || e.Shift)
		{
			m_speedSlow = false;
		}
	}
	
	void CameraController::OnMouseMovedRaw(const Elos::Event::MouseMovedRaw& e)
	{
		const f32 deltaX = static_cast<f32>(e.DeltaX);
		const f32 deltaY = static_cast<f32>(e.DeltaY);
	
		if (m_rotating)
		{
			const f32 rotX = DirectX::XMConvertToRadians(deltaX * m_settings.RotationSpeed);
			const f32 rotY = DirectX::XMConvertToRadians(deltaY * m_settings.RotationSpeed);

			// Update yaw (horizontal rotation)
			m_yaw -= rotX;

			// Update pitch (vertical rotation) with clamping to prevent flipping
			m_pitch -= rotY;
			m_pitch = std::clamp(m_pitch, -DirectX::XM_PIDIV2 * 0.99f, DirectX::XM_PIDIV2 * 0.99f);

			UpdateCameraTransform();
		}
		else if (m_panning && !m_settings.FirstPersonMode)
		{
			const f32 panSpeed = 0.01f * m_settings.OrbitDistance;

			Vector3 right = m_camera->GetRightVector();
			Vector3 up = m_camera->GetUpVector();

			m_settings.OrbitPoint -= right * (deltaX * panSpeed);
			m_settings.OrbitPoint += up * (deltaY * panSpeed);

			UpdateCameraTransform();
		}

	}
	
	void CameraController::OnMouseButtonPressed(const Elos::Event::MouseButtonPressed& e)
	{
		if (e.Button == m_settings.RotateMouseButton)
		{
			m_rotating = true;
		}
		else if (e.Button == m_settings.PanMouseButton)
		{
			m_panning = true;
		}
	}
	
	void CameraController::OnMouseButtonReleased(const Elos::Event::MouseButtonReleased& e)
	{
		if (e.Button == m_settings.RotateMouseButton)
		{
			m_rotating = false;
		}
		else if (e.Button == m_settings.PanMouseButton)
		{
			m_panning = false;
		}
	}
	
	void CameraController::OnMouseWheelScrolled(const Elos::Event::MouseWheelScrolled& e)
	{
		if (m_settings.FirstPersonMode)
		{
			m_camera->ZoomBy(e.Delta * m_settings.ZoomSensitivity);
		}
		else
		{
			// In orbit mode, control orbit distance
			const f32 zoomFactor = 1.0f - (e.Delta * m_settings.ZoomSensitivity);
			m_targetOrbitDistance *= zoomFactor;

			const f32 minDistance = 0.1f;
			const f32 maxDistance = 1000.0f;
			m_targetOrbitDistance = std::clamp(m_targetOrbitDistance, minDistance, maxDistance);
		}
	}
	
	void CameraController::SetSettings(const Settings& settings)
	{
		m_settings            = settings;
		m_targetOrbitDistance = settings.OrbitDistance;
	}
	
	void CameraController::SetFirstPersonMode(bool enabled)
	{
		if (m_settings.FirstPersonMode == enabled)
		{
			return;
		}

		m_settings.FirstPersonMode = enabled;
		Vector3 currentCameraPos = m_camera->GetPosition();
		Vector3 currentForward = m_camera->GetForwardVector();

		if (!enabled)
		{
			// Switching to orbit mode
			// Calculate a sensible orbit point in front of the camera
			constexpr f32 distanceInFront = 5.0f;
			m_settings.OrbitPoint = currentCameraPos + currentForward * distanceInFront;

			// Set orbit distance based on current position and new orbit point
			Vector3 toOrbit = m_settings.OrbitPoint - currentCameraPos;
			m_settings.OrbitDistance = toOrbit.Length();
			m_targetOrbitDistance = m_settings.OrbitDistance;

			// Reset pitch and yaw based on current camera orientation
			// Correctly handle the orbit conversion
			Vector3 dirToOrbit = toOrbit;
			dirToOrbit.Normalize();

			m_yaw = std::atan2f(dirToOrbit.x, dirToOrbit.z);

			// Calculate pitch from dot product of dirToOrbit and up vector
			f32 dotUp = dirToOrbit.Dot(Vector3::Up);
			m_pitch = std::asinf(dotUp);
		}
		else
		{
			// Switching to first-person mode
			// Preserve current view direction
			m_yaw = std::atan2f(currentForward.x, currentForward.z);
			m_pitch = std::asinf(currentForward.y);

			// Keep current position
			m_targetPosition = currentCameraPos;
		}

		// Force an update to apply the new mode
		UpdateCameraTransform();
	}
	
	void CameraController::SetOrbitPoint(const Vector3& point)
	{
		m_settings.OrbitPoint = point;
		if (!m_settings.FirstPersonMode)
		{
			UpdateCameraTransform();
		}
	}
	
	void CameraController::SetOrbitDistance(f32 distance)
	{
		m_settings.OrbitDistance = std::max(0.1f, distance);
		m_targetOrbitDistance    = m_settings.OrbitDistance;
		if (!m_settings.FirstPersonMode)
		{
			UpdateCameraTransform();
		}
	}
	
	void CameraController::ResetCamera()
	{
		if (m_settings.FirstPersonMode)
		{
			m_camera->SetPosition(Vector3(0.0f, 5.0f, -15.0f));
			m_camera->SetLookAt(Vector3::Zero);
			m_targetPosition = m_camera->GetPosition();
		}
		else
		{
			m_settings.OrbitPoint = Vector3::Zero;
			m_settings.OrbitDistance = 10.0f;
			m_targetOrbitDistance = m_settings.OrbitDistance;
			m_pitch = DirectX::XMConvertToRadians(30.0f); // 30 degrees down
			m_yaw = 0.0f;
		}

		m_smoothedVelocity = Vector3::Zero;
		UpdateCameraTransform();
	}

	void CameraController::UpdateFirstPersonCamera(f64 deltaTime)
	{

		f32 currentSpeed = m_settings.MoveSpeed;
		if (m_speedBoost)
		{
			currentSpeed *= 2.0f;
		}
		if (m_speedSlow)
		{
			currentSpeed *= 0.5f;
		}

		// Calculate movement direction
		Vector3 moveDirection = Vector3::Zero;

		// Debug print movement state
		bool anyMovement = false;

		if (m_movingForward)
		{
			moveDirection += m_camera->GetForwardVector();
			anyMovement = true;
		}
		if (m_movingBackward)
		{
			moveDirection -= m_camera->GetForwardVector();
			anyMovement = true;
		}
		if (m_movingRight)
		{
			moveDirection += m_camera->GetRightVector();
			anyMovement = true;
		}
		if (m_movingLeft)
		{
			moveDirection -= m_camera->GetRightVector();
			anyMovement = true;
		}
		if (m_movingUp)
		{
			moveDirection += m_camera->GetUpVector();
			anyMovement = true;
		}
		if (m_movingDown)
		{
			moveDirection -= m_camera->GetUpVector();
			anyMovement = true;
		}

		if (moveDirection.LengthSquared() > kEpsilon)
		{
			moveDirection.Normalize();
		}


		// Calculate new position
		m_targetPosition = m_camera->GetPosition() + moveDirection * currentSpeed * static_cast<f32>(deltaTime);

		// Apply position smoothing
		if (m_settings.MovementSmoothingFactor > 0.0f)
		{
			// Convert smoothing factor to a time value for SmoothDamp
			const f32 smoothTime = m_settings.MovementSmoothingFactor;

			// Simple smooth damp implementation
			const Vector3 currentPos = m_camera->GetPosition();
			const Vector3 delta = m_targetPosition - currentPos;

			// Update velocity using a spring-damper system
			m_smoothedVelocity += delta * (1.0f / smoothTime) * static_cast<f32>(deltaTime);
			m_smoothedVelocity *= std::max(0.0f, 1.0f - (3.0f * static_cast<f32>(deltaTime) / smoothTime));

			// Apply velocity to position
			Vector3 newPosition = currentPos + m_smoothedVelocity * static_cast<f32>(deltaTime);
			m_camera->SetPosition(newPosition);
		}
		else
		{
			m_camera->SetPosition(m_targetPosition);
		}

		if (anyMovement)
		{
			UpdateCameraTransform();
		}
	}
	
	void CameraController::UpdateOrbitCamera(f64 deltaTime)
	{
		if (std::abs(m_settings.OrbitDistance - m_targetOrbitDistance) > 0.01f)
		{
			m_settings.OrbitDistance += (m_targetOrbitDistance - m_settings.OrbitDistance) *
				std::min(1.0f, static_cast<f32>(deltaTime) * 5.0f);
		}
		else
		{
			m_settings.OrbitDistance = m_targetOrbitDistance;
		}

		UpdateCameraTransform();
	}
	
	void CameraController::UpdateCameraTransform()
	{
		if (m_settings.FirstPersonMode)
		{
			// Calculate new forward direction from pitch and yaw
			Vector3 forward;
			forward.x = std::sinf(m_yaw) * std::cosf(m_pitch);
			forward.y = std::sinf(m_pitch);
			forward.z = std::cosf(m_yaw) * std::cosf(m_pitch);

			// Calculate right and up vectors to maintain orthogonal basis
			Vector3 right = Vector3::Up.Cross(forward);
			if (right.LengthSquared() < kEpsilon)
			{
				// Handle case when forward is parallel to world up
				right = Vector3::Right;
			}
			else
			{
				right.Normalize();
			}

			Vector3 up = forward.Cross(right);
			up.Normalize();

			m_camera->SetLookAt(m_camera->GetPosition() + forward);
		}
		else
		{
			// Calculate camera position in orbit mode
			Vector3 cameraPos;
			cameraPos.x = m_settings.OrbitPoint.x + m_settings.OrbitDistance * std::sinf(m_yaw) * std::cosf(m_pitch);
			cameraPos.y = m_settings.OrbitPoint.y + m_settings.OrbitDistance * std::sinf(m_pitch);
			cameraPos.z = m_settings.OrbitPoint.z + m_settings.OrbitDistance * std::cosf(m_yaw) * std::cosf(m_pitch);

			m_camera->SetPosition(cameraPos);
			m_camera->SetLookAt(m_settings.OrbitPoint);
		}

		m_camera->Update();
	}
}