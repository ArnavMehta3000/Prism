#include "SceneNodeFactory.h"
#include "Application/Scene/Scene.h"
#include "Application/Scene/SceneNode.h"
#include "Application/Scene/AnimatedSceneNode.h"
#include "Graphics/Mesh.h"
#include "Utils/Log.h"
#include <array>

namespace Prism
{
	SceneNode* SceneNodeFactory::CreateMeshNode(
		Scene& scene, 
		const Elos::String& name, 
		std::shared_ptr<Gfx::Mesh> mesh, 
		const Vector3& position, 
		const Quaternion& rotation, 
		const Vector3& scale, 
		SceneNode* parent)
	{
		SceneNode* node = scene.CreateNode(name, parent);
		if (!node)
		{
			Log::Error("Failed to create node '{}'", name);
			return nullptr;
		}

		node->SetPosition(position);
		node->SetRotation(rotation);
		node->SetScale(scale);

		SceneNode::RenderData renderData;
		renderData.Mesh = mesh;
		node->SetRenderData(renderData);

		return node;
	}
	
	AnimatedSceneNode* SceneNodeFactory::CreateRotatingNode(Scene& scene, 
		const Elos::String& name, 
		std::shared_ptr<Gfx::Mesh> mesh, 
		f32 rotationSpeed, 
		const Vector3& rotationAxis, 
		const Vector3& position, 
		const Quaternion& initialRotation, 
		const Vector3& scale, 
		SceneNode* parent)
	{
		if (scene.FindNode(name))
		{
			Log::Warn("Node with name '{}' already exists", name);
			return nullptr;
		}

		auto animatedNode = std::make_unique<AnimatedSceneNode>(name);
		AnimatedSceneNode* nodePtr = animatedNode.get();

		nodePtr->SetPosition(position);
		nodePtr->SetRotation(initialRotation);
		nodePtr->SetScale(scale);

		SceneNode::RenderData renderData;
		renderData.Mesh = mesh;
		nodePtr->SetRenderData(renderData);

		nodePtr->SetAnimationFunction(AnimatedSceneNode::CreateRotationAnimation(rotationSpeed, rotationAxis));

		// Add to parent (or root)
		if (parent)
		{
			parent->AddChild(std::move(animatedNode));
		}
		else
		{
			scene.GetRootNode()->AddChild(std::move(animatedNode));
		}

		return nodePtr;
	}
	
	AnimatedSceneNode* SceneNodeFactory::CreateOrbitingNode(
		Scene& scene, 
		const Elos::String& name, 
		std::shared_ptr<Gfx::Mesh> mesh, 
		const Vector3& centerPoint, 
		f32 radius, 
		f32 orbitSpeed, 
		const Vector3& orbitAxis, 
		const Vector3& scale, 
		SceneNode* parent)
	{
		if (scene.FindNode(name))
		{
			Log::Warn("Node with name '{}' already exists", name);
			return nullptr;
		}

		auto animatedNode = std::make_unique<AnimatedSceneNode>(name);
		AnimatedSceneNode* nodePtr = animatedNode.get();

		// Initial position doesn't matter as it will be set by the animation
		// but we'll set the scale
		nodePtr->SetScale(scale);

		SceneNode::RenderData renderData;
		renderData.Mesh = mesh;
		nodePtr->SetRenderData(renderData);

		// Set animation
		nodePtr->SetAnimationFunction(AnimatedSceneNode::CreateOrbitAnimation(radius, orbitSpeed, centerPoint, orbitAxis));

		if (parent)
		{
			parent->AddChild(std::move(animatedNode));
		}
		else
		{
			scene.GetRootNode()->AddChild(std::move(animatedNode));
		}

		return nodePtr;
	}

	SceneNode* SceneNodeFactory::CreateSolarSystem(Scene& scene, std::shared_ptr<Gfx::Mesh> sphereMesh, SceneNode* parent)
	{
		SceneNode* sunNode = CreateMeshNode(
			scene,
			"Sun",
			sphereMesh,
			Vector3::Zero,
			Quaternion::Identity,
			Vector3(2.0f, 2.0f, 2.0f),
			parent);

		if (!sunNode)
		{
			Log::Error("Failed to create sun node");
			return nullptr;
		}

		AnimatedSceneNode* animatedSun = dynamic_cast<AnimatedSceneNode*>(sunNode);
		if (animatedSun)
		{
			animatedSun->SetAnimationFunction(
				AnimatedSceneNode::CreateRotationAnimation(0.2f, Vector3::Up));
		}

		struct PlanetInfo
		{
			Elos::String Name;
			f32 Radius;
			f32 OrbitSpeed;
			f32 Size;
			Vector3 OrbitAxis;
		};

		const std::array<PlanetInfo, 4> planets = 
		{ 
			{
				{ "Planet1", 4.0f, 0.5f, 0.5f, Vector3::Up },
				{ "Planet2", 6.0f, 0.3f, 0.7f, Vector3::Up },
				{ "Planet3", 8.0f, 0.2f, 0.8f, Vector3(0.0f, 0.9f, 0.1f) },
				{ "Planet4", 10.0f, 0.1f, 0.9f, Vector3(0.1f, 1.0f, 0.0f) }
			} 
		};


		for (const PlanetInfo& planetInfo : planets)
		{
			AnimatedSceneNode* planet = CreateOrbitingNode(
				scene,
				planetInfo.Name,
				sphereMesh,
				Vector3::Zero,
				planetInfo.Radius,
				planetInfo.OrbitSpeed,
				planetInfo.OrbitAxis,
				Vector3(planetInfo.Size, planetInfo.Size, planetInfo.Size),
				sunNode);

			if (!planet)
			{
				Log::Warn("Failed to create planet '{}'", planetInfo.Name);
				continue;
			}

			// Add a moon to some planets
			if (planetInfo.Name == "Planet2" || planetInfo.Name == "Planet4")
			{
				const float moonRadius = 1.5f;
				const float moonScale = 0.3f;
				const float moonSpeed = 1.0f;

				std::string moonName = planetInfo.Name + "_Moon";

				CreateOrbitingNode(
					scene,
					moonName,
					sphereMesh,
					Vector3::Zero, // Will be relative to the planet
					moonRadius,
					moonSpeed,
					Vector3::Up,
					Vector3(moonScale, moonScale, moonScale),
					planet);
			}
		}

		return sunNode;
	}
}
