#include "AnimatedSceneNode.h"

namespace Prism
{
	AnimatedSceneNode::AnimatedSceneNode(const std::string& name) : SceneNode(name)
	{
	}
	
	void AnimatedSceneNode::Update(const f32 deltaTime)
	{
		if (m_animFunc)
		{
			m_animFunc(*this, deltaTime);
		}

		SceneNode::Update(deltaTime);
	}
	
	AnimatedSceneNode::AnimationUpdateFunc AnimatedSceneNode::CreateRotationAnimation(f32 speed, const Vector3& axis)
	{
		return [speed, axis](AnimatedSceneNode& node, float deltaTime)
		{
			Quaternion rotationDelta = Quaternion::CreateFromAxisAngle(axis, speed * deltaTime);
			node.SetRotation(rotationDelta * node.GetRotation());
		};
	}
	
	AnimatedSceneNode::AnimationUpdateFunc AnimatedSceneNode::CreateOscillationAnimation(f32 amplitude, f32 frequency, const Vector3& axis)
	{
		// This captures the current time so we can calculate the oscillation
		static f32 totalTime = 0.0f;

		return [amplitude, frequency, axis](AnimatedSceneNode& node, f32 deltaTime) mutable
		{
			totalTime += deltaTime;

			const f32 displacement = amplitude * std::sin(frequency * totalTime);

			Vector3 newPosition = node.GetPosition();
			newPosition += axis * displacement * deltaTime;

			node.SetPosition(newPosition);
		};
	}
	
	AnimatedSceneNode::AnimationUpdateFunc AnimatedSceneNode::CreateOrbitAnimation(f32 radius, f32 speed, const Vector3& center, const Vector3& axis)
	{
		// This captures the current angle so we can update it
		static float currentAngle = 0.0f;
		Vector3 normalizedAxis;
		axis.Normalize(normalizedAxis);

		return [radius, speed, center, normalizedAxis](AnimatedSceneNode& node, f32 deltaTime) mutable
		{
			// Update the angle
			currentAngle += speed * deltaTime;

			Matrix rotation = Matrix::CreateFromAxisAngle(normalizedAxis, currentAngle);
			Vector3 offset = Vector3(radius, 0.0f, 0.0f);

			// Rotate this offset and add to the center
			offset = Vector3::Transform(offset, rotation);
			Vector3 newPosition = center + offset;

			node.SetPosition(newPosition);

			Vector3 lookDirection = center - newPosition;
			if (lookDirection.LengthSquared() > 0.001f) // Avoid division by zero
			{
				lookDirection.Normalize();

				// Convert look direction to a rotation
				// This simplified approach assumes we're looking along the Z axis by default
				Vector3 up = Vector3::Up;
				Vector3 right = up.Cross(lookDirection);
				right.Normalize();
				up = lookDirection.Cross(right);

				Matrix lookAtMatrix = Matrix::CreateWorld(newPosition, -lookDirection, up);
				node.SetRotation(Quaternion::CreateFromRotationMatrix(lookAtMatrix));
			}
		};
	}
}