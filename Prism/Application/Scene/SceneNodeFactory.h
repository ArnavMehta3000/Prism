#pragma once
#include "StandardTypes.h"
#include "Math/Math.h"
#include <Elos/Common/String.h>
#include <Elos/Common/FunctionMacros.h>
#include <memory>

namespace Prism
{
    class Scene;
    class SceneNode;
    class AnimatedSceneNode;

    namespace Gfx
    {
        class Mesh;
    }

    class SceneNodeFactory
    {
    public:
        static NODISCARD SceneNode* CreateMeshNode(
            Scene& scene,
            const Elos::String& name,
            std::shared_ptr<Gfx::Mesh> mesh,
            const Vector3& position    = Vector3::Zero,
            const Quaternion& rotation = Quaternion::Identity,
            const Vector3& scale       = Vector3::One,
            SceneNode* parent          = nullptr);

        // Create an animated node with rotation animation
        static NODISCARD AnimatedSceneNode* CreateRotatingNode(
            Scene& scene,
            const Elos::String& name,
            std::shared_ptr<Gfx::Mesh> mesh,
            f32 rotationSpeed,
            const Vector3& rotationAxis       = Vector3::Up,
            const Vector3& position           = Vector3::Zero,
            const Quaternion& initialRotation = Quaternion::Identity,
            const Vector3& scale              = Vector3::One,
            SceneNode* parent                 = nullptr);

        // Create a node that orbits around a point
        static AnimatedSceneNode* CreateOrbitingNode(
            Scene& scene,
            const Elos::String& name,
            std::shared_ptr<Gfx::Mesh> mesh,
            const Vector3& centerPoint,
            f32 radius,
            f32 orbitSpeed,
            const Vector3& orbitAxis = Vector3::Up,
            const Vector3& scale     = Vector3::One,
            SceneNode* parent        = nullptr);

        // Create a solar system demonstration
        static NODISCARD SceneNode* CreateSolarSystem(
            Scene& scene,
            std::shared_ptr<Gfx::Mesh> sphereMesh,
            SceneNode* parent = nullptr);
    };
}