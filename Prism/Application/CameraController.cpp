#include "CameraController.h"

namespace Prism
{
	CameraController::CameraController(Gfx::Camera* camera, const Settings& settings)
		: m_camera(camera)
		, m_settings(settings)
		, m_position(camera->GetPosition())
		, m_rotation(Quaternion::Identity)
	{
		// Initialize rotation from camera's current orientation
		Vector3 forward = m_camera->GetForwardVector();
		Vector3 up = m_camera->GetUpVector();

		// Create rotation quaternion from look direction
		Vector3 zAxis = -forward;
		Vector3 xAxis = up.Cross(zAxis);
		xAxis.Normalize();
		Vector3 yAxis = zAxis.Cross(xAxis);
		yAxis.Normalize();

		// Construct rotation matrix and convert to quaternion
		const Matrix rotMatrix(
			xAxis.x, xAxis.y, xAxis.z, 0.0f,
			yAxis.x, yAxis.y, yAxis.z, 0.0f,
			zAxis.x, zAxis.y, zAxis.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		m_rotation = Quaternion::CreateFromRotationMatrix(rotMatrix);

		// Extract euler angles (for input handling)
		Matrix rotationMatrix = Matrix::CreateFromQuaternion(m_rotation);

		// Extract pitch (rotation around X)
		m_euler.x = std::asinf(-rotationMatrix._32);

		// Extract yaw (rotation around Y)
		m_euler.y = (std::abs(rotationMatrix._32) < 0.99999f) ? 
			std::atan2f(rotationMatrix._31, rotationMatrix._33) :
			std::atan2f(-rotationMatrix._13, rotationMatrix._11);

		// No rolling in FPS camera
		m_euler.z = 0.0f;

		UpdateCameraTransform();
	}
	
	void CameraController::Update(const Elos::Timer::TimeInfo& timeInfo)
	{
		UpdateCameraMovement(static_cast<f32>(timeInfo.DeltaTime));
		UpdateCameraOrientation();
		UpdateCameraTransform();
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
	}

	void CameraController::OnKeyReleased(const Elos::Event::KeyReleased& e)
	{
		if (e.Key == m_settings.ForwardKey)
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
		if (m_rotating)
		{
			// Convert mouse movement to rotation
			const f32 yawDelta   = -DirectX::XMConvertToRadians(e.DeltaX * m_settings.RotationSpeed);
			const f32 pitchDelta = -DirectX::XMConvertToRadians(e.DeltaY * m_settings.RotationSpeed);

			m_euler.y += yawDelta;
			m_euler.x += pitchDelta;

			// Clamp pitch to prevent flipping
			m_euler.x = std::clamp(m_euler.x, -DirectX::XM_PIDIV2 * 0.99f, DirectX::XM_PIDIV2 * 0.99f);

			// Normalize yaw for numerical stability
			if (m_euler.y > DirectX::XM_PI)
			{
				m_euler.y -= DirectX::XM_2PI;
			}
			else if (m_euler.y < -DirectX::XM_PI)
			{
				m_euler.y += DirectX::XM_2PI;
			}
		}
	}

	void CameraController::OnMouseButtonPressed(const Elos::Event::MouseButtonPressed& e)
	{
		if (e.Button == m_settings.RotateMouseButton)
		{
			m_rotating = true;
		}
	}

	void CameraController::OnMouseButtonReleased(const Elos::Event::MouseButtonReleased& e)
	{
		if (e.Button == m_settings.RotateMouseButton)
		{
			m_rotating = false;
		}
	}

	void CameraController::OnMouseWheelScrolled(const Elos::Event::MouseWheelScrolled& e)
	{
		m_camera->ZoomBy(e.Delta * m_settings.ZoomSensitivity);
	}

	void CameraController::ResetCamera()
	{
		m_position = Vector3(0.0f, 0.0f, 15.0f);
		m_euler = Vector3::Zero;

		UpdateCameraOrientation();

		// Reset velocity
		m_currentVelocity = Vector3::Zero;
		m_targetVelocity = Vector3::Zero;

		UpdateCameraTransform();
	}

	void CameraController::SetPosition(const Vector3& position)
	{
		m_position = position;
		UpdateCameraTransform();
	}

	void CameraController::SetRotation(const Quaternion& rotation)
	{
		m_rotation = rotation;

		const Matrix rotationMatrix = Matrix::CreateFromQuaternion(m_rotation);

		m_euler.x = std::asinf(-rotationMatrix._32);
		m_euler.y = (std::abs(rotationMatrix._32) < 0.99999f) ?
			std::atan2f(rotationMatrix._31, rotationMatrix._33) :
			std::atan2f(-rotationMatrix._13, rotationMatrix._11);
		m_euler.z = 0.0f;

		UpdateCameraTransform();
	}
	
	void CameraController::UpdateCameraOrientation()
	{
		m_rotation = Quaternion::CreateFromYawPitchRoll(m_euler);
	}

	void CameraController::UpdateCameraMovement(const f32 deltaTime)
	{
		const Vector3 moveDir = CalculateMovementDirection();
		const bool hasInput   = moveDir.LengthSquared() > kEpsilon;

		// Only update target velocity with input when right mouse button is pressed
		if (m_rotating && hasInput)
		{
			const f32 currentSpeed = CalculateCurrentMoveSpeed();
			m_targetVelocity       = moveDir * currentSpeed;
			m_wasMovingLastFrame   = true;
		}
		else
		{
			// When not rotating or no movement input, set target to zero
			m_targetVelocity = Vector3::Zero;

			// Only remember we were moving if we actually had movement
			if (m_rotating)
			{
				m_wasMovingLastFrame = hasInput;
			}
		}

		// Always apply smoothing - this handles deceleration
		SmoothDamp(m_currentVelocity, m_targetVelocity, m_currentVelocity,
			m_settings.MovementSmoothTime, deltaTime);

		if (m_currentVelocity.LengthSquared() > kEpsilon)
		{
			m_position += m_currentVelocity * deltaTime;
			m_camera->SetPosition(m_position);
		}
	}

	void CameraController::UpdateCameraTransform()
	{
		m_camera->SetPosition(m_position);
		m_camera->SetOrientation(m_rotation);
		m_camera->Update();
	}
	
	Vector3 CameraController::CalculateMovementDirection() const
	{
		Vector3 moveDir = Vector3::Zero;

		// Get the camera's basis vectors
		Vector3 forward = m_camera->GetForwardVector();
		Vector3 right   = m_camera->GetRightVector();
		Vector3 up      = m_camera->GetUpVector();

		// Remove any up/down component from forward vector for traditional FPS movement
		if (forward.y != 0.0f)
		{
			forward.y = 0.0f;
			if (forward.LengthSquared() > kEpsilon)
			{
				forward.Normalize();
			}
		}

		// Accumulate movement direction
		if (m_movingForward)
		{
			moveDir += forward;
		}
		if (m_movingBackward)
		{
			moveDir -= forward;
		}
		if (m_movingRight)
		{
			moveDir += right;
		}
		if (m_movingLeft)
		{
			moveDir -= right;
		}
		if (m_movingUp)
		{
			moveDir += up;
		}
		if (m_movingDown)
		{
			moveDir -= up;
		}

		if (moveDir.LengthSquared() > kEpsilon)
		{
			moveDir.Normalize();
		}

		return moveDir;
	}
	
	f32 CameraController::CalculateCurrentMoveSpeed() const
	{
		f32 speed = m_settings.MoveSpeed;

		if (m_speedBoost)
		{
			speed *= m_settings.SpeedBoostFactor;
		}
		
		if (m_speedSlow)
		{
			speed *= m_settings.SpeedSlowFactor;
		}

		return speed;
	}
	
	void CameraController::SmoothDamp(Vector3& current, const Vector3& target, Vector3& velocity, f32 smoothTime, const f32 deltaTime)
	{
		smoothTime = std::max(0.0001f, smoothTime);

		f32 damping = 1.0f;
		if (!m_rotating || m_targetVelocity.LengthSquared() < kEpsilon)
		{
			damping = m_settings.MovementDamping;
		}

		if (!m_rotating && m_wasMovingLastFrame && m_targetVelocity.LengthSquared() < kEpsilon)
		{
			damping *= 1.5f;
		}

		const f32 omega = 2.0f / std::max(smoothTime, 0.01f);
		const f32 x = omega * deltaTime * damping;
		const f32 exp = 1.0f / (1.0f + x + 0.5f * x * x + 0.25f * x * x * x);

		const Vector3 delta = current - target;
		const Vector3 originalTo = target;

		const Vector3 temp = (velocity + omega * delta) * deltaTime;
		velocity = (velocity - omega * temp) * exp;
		current = target + (delta + temp) * exp;

		// Overshooting check
		if ((current - originalTo).Dot(target - originalTo) > 0)
		{
			current = originalTo;
			velocity = Vector3::Zero;
		}

		// Add a velocity threshold
		const f32 velocityThreshold = 0.05f;
		if (velocity.LengthSquared() < velocityThreshold * velocityThreshold)
		{
			velocity = Vector3::Zero;
		}
	}
}