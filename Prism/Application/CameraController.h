#pragma once
#include "Application/AppEvents.h"
#include "Graphics/Camera.h"
#include <Elos/Utils/Timer.h>
#include <Elos/Window/Input/KeyCode.h>

namespace Prism
{
    class CameraController
    {
    public:
        struct Settings
        {
            // Movement speed in units per second
            f32 MoveSpeed = 5.0f;
            // Rotation speed in degrees per pixel
            f32 RotationSpeed = 0.2f;
            // Mouse wheel zoom sensitivity
            f32 ZoomSensitivity = 0.1f;
            // Smoothing factor for camera movement (0 = no smoothing, 1 = max smoothing)
            f32 MovementSmoothingFactor = 0.2f;
            // Smoothing factor for camera rotation (0 = no smoothing, 1 = max smoothing)
            f32 RotationSmoothingFactor = 0.1f;
            // Whether to use first-person controls (true) or orbit controls (false)
            bool FirstPersonMode = true;
            // Orbit point for orbit mode
            Vector3 OrbitPoint = Vector3::Zero;
            // Orbit distance for orbit mode
            f32 OrbitDistance = 10.0f;

            Elos::KeyCode::Key ForwardKey                = Elos::KeyCode::W;
            Elos::KeyCode::Key BackwardKey               = Elos::KeyCode::S;
            Elos::KeyCode::Key LeftKey                   = Elos::KeyCode::A;
            Elos::KeyCode::Key RightKey                  = Elos::KeyCode::D;
            Elos::KeyCode::Key UpKey                     = Elos::KeyCode::E;
            Elos::KeyCode::Key DownKey                   = Elos::KeyCode::Q;
            Elos::KeyCode::Key OrbitToggleKey            = Elos::KeyCode::R;
            Elos::KeyCode::Key SpeedBoostKey             = Elos::KeyCode::LControl;
            Elos::KeyCode::Key SpeedSlowKey              = Elos::KeyCode::LShift;
            Elos::KeyCode::MouseButton RotateMouseButton = Elos::KeyCode::MouseButton::Right;
            Elos::KeyCode::MouseButton PanMouseButton    = Elos::KeyCode::MouseButton::Middle;
        };

    public:
        CameraController(Gfx::Camera* camera, const Settings& settings = Settings{});
        ~CameraController() = default;

        void Update(const Elos::Timer::TimeInfo& timeInfo);

        void OnKeyPressed(const Elos::Event::KeyPressed& e);
        void OnKeyReleased(const Elos::Event::KeyReleased& e);
        void OnMouseMovedRaw(const Elos::Event::MouseMovedRaw& e);
        void OnMouseButtonPressed(const Elos::Event::MouseButtonPressed& e);
        void OnMouseButtonReleased(const Elos::Event::MouseButtonReleased& e);
        void OnMouseWheelScrolled(const Elos::Event::MouseWheelScrolled& e);

        void SetSettings(const Settings& settings);
        NODISCARD inline Settings& GetSettings() { return m_settings; }

        void SetFirstPersonMode(bool enabled);
        void SetOrbitPoint(const Vector3& point);
        void SetOrbitDistance(f32 distance);

        void ResetCamera();

    private:
        void UpdateFirstPersonCamera(f64 deltaTime);
        void UpdateOrbitCamera(f64 deltaTime);
        void UpdateCameraTransform();

    private:
        Settings     m_settings;
        Gfx::Camera* m_camera         = nullptr;
        bool         m_movingForward  = false;
        bool         m_movingBackward = false;
        bool         m_movingLeft     = false;
        bool         m_movingRight    = false;
        bool         m_movingUp       = false;
        bool         m_movingDown     = false;
        bool         m_speedBoost     = false;
        bool         m_speedSlow      = false;
        bool         m_rotating       = false;
        bool         m_panning        = false;
        f32          m_pitch          = 0.0f;
        f32          m_yaw            = 0.0f;
        Vector3      m_targetPosition;
        Vector3      m_smoothedVelocity;
        f32          m_targetOrbitDistance;
    };
}