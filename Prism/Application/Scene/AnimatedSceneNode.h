#pragma once
#include "Application/Scene/SceneNode.h"
#include <functional>

namespace Prism
{
    class AnimatedSceneNode : public SceneNode
    {
    public:
        using AnimationUpdateFunc = std::function<void(AnimatedSceneNode&, f32 deltaTime)>;

        AnimatedSceneNode(const std::string& name);
        ~AnimatedSceneNode() override = default;

        void SetAnimationFunction(AnimationUpdateFunc animFunc) { m_animFunc = std::move(animFunc); }
        void Update(const f32 deltaTime) override;

        static NODISCARD AnimationUpdateFunc CreateRotationAnimation(f32 speed, const Vector3& axis);
        static NODISCARD AnimationUpdateFunc CreateOscillationAnimation(f32 amplitude, f32 frequency, const Vector3& axis);
        static NODISCARD AnimationUpdateFunc CreateOrbitAnimation(f32 radius, f32 speed, const Vector3& center, const Vector3& axis);

    private:
        AnimationUpdateFunc m_animFunc;
    };
}