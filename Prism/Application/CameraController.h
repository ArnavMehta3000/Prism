#pragma once
#include "Graphics/Camera.h"
#include "Application/AppEvents.h"
#include <Elos/Utils/Timer.h>
#include <Elos/Window/Input/KeyCode.h>

namespace Prism
{
	class CameraController
	{
	public:
		struct Settings
		{
			f32 MoveSpeed          = 8.0f;          // Base movement speed in units per second
			f32 MovementSmoothTime = 0.2f;          // Time to reach target velocity (in seconds)
			f32 MovementDamping    = 1.5f;          // Additional damping factor when no input
			f32 RotationSpeed      = 0.2f;          // Rotation speed in degrees per pixel
			f32 ZoomSensitivity    = 0.1f;          // Mouse wheel zoom sensitivity
			f32 SpeedBoostFactor   = 2.0f;          // Multiplier when boost key is pressed
			f32 SpeedSlowFactor    = 0.5f;          // Multiplier when slow key is pressed

			Elos::KeyCode::Key ForwardKey                = Elos::KeyCode::W;
			Elos::KeyCode::Key BackwardKey               = Elos::KeyCode::S;
			Elos::KeyCode::Key LeftKey                   = Elos::KeyCode::A;
			Elos::KeyCode::Key RightKey                  = Elos::KeyCode::D;
			Elos::KeyCode::Key UpKey                     = Elos::KeyCode::E;
			Elos::KeyCode::Key DownKey                   = Elos::KeyCode::Q;
			Elos::KeyCode::Key SpeedBoostKey             = Elos::KeyCode::LControl;
			Elos::KeyCode::Key SpeedSlowKey              = Elos::KeyCode::LShift;
			Elos::KeyCode::MouseButton RotateMouseButton = Elos::KeyCode::MouseButton::Right;
		};

	public:
		CameraController(Gfx::Camera* camera, const Settings& settings = Settings{});
		~CameraController() = default;

		inline NODISCARD Settings& GetSettings() { return m_settings; }
		inline NODISCARD const Vector3& GetCurrentVelocity() const { return m_currentVelocity; }
		inline void SetSettings(const Settings& settings) { m_settings = settings; }

		void Update(const Elos::Timer::TimeInfo& timeInfo);
		void OnKeyPressed(const Elos::Event::KeyPressed& e);
		void OnKeyReleased(const Elos::Event::KeyReleased& e);
		void OnMouseMovedRaw(const Elos::Event::MouseMovedRaw& e);
		void OnMouseButtonPressed(const Elos::Event::MouseButtonPressed& e);
		void OnMouseButtonReleased(const Elos::Event::MouseButtonReleased& e);
		void OnMouseWheelScrolled(const Elos::Event::MouseWheelScrolled& e);
		void ResetCamera();
		void SetPosition(const Vector3& position);
		void SetRotation(const Quaternion& rotation);

	private:
		void UpdateCameraOrientation();
		void UpdateCameraMovement(const f32 deltaTime);
		void UpdateCameraTransform();
		Vector3 CalculateMovementDirection() const;
		f32 CalculateCurrentMoveSpeed() const;
		void SmoothDamp(Vector3& current, const Vector3& target, Vector3& velocity, f32 smoothTime, const f32 deltaTime);

	private:
		Settings     m_settings;
		Gfx::Camera* m_camera             = nullptr;
		bool         m_movingForward      = false;
		bool         m_movingBackward     = false;
		bool         m_movingLeft         = false;
		bool         m_movingRight        = false;
		bool         m_movingUp           = false;
		bool         m_movingDown         = false;
		bool         m_speedBoost         = false;
		bool         m_speedSlow          = false;
		bool         m_rotating           = false;
		bool         m_wasMovingLastFrame = false;
		Vector3      m_position;
		Quaternion   m_rotation;
		Vector3      m_euler;
		Vector3      m_currentVelocity;
		Vector3      m_targetVelocity;
	};
}