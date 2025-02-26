#include "SceneNode.h"
#include "Utils/Log.h"
#include "Graphics/Camera.h"
#include "Graphics/Renderer.h"
#include "Graphics/Mesh.h"
#include <Elos/Common/Assert.h>
#include <algorithm>

namespace Prism
{
	SceneNode::SceneNode(const Elos::String& name)
		: m_name(name)
	{
	}
	
	SceneNode::~SceneNode() = default;
	
	void SceneNode::AddChild(std::unique_ptr<SceneNode> child)
	{
		if (!child)
		{
			return;
		}

		// Set parent relationship
		child->m_parent = this;
		m_children.push_back(std::move(child));
	}
	
	void SceneNode::RemoveChild(SceneNode* child)
	{
		if (!child)
		{
			return;
		}

		// Find the child in our collection
		auto it = std::find_if(m_children.begin(), m_children.end(),
		[child](const std::unique_ptr<SceneNode>& node)
		{
			return node.get() == child;
		});

		if (it != m_children.end())
		{
			// Break the parent relationship
			(*it)->m_parent = nullptr;

			// Move the child out of our collection (transferring ownership)
			std::unique_ptr<SceneNode> orphanedChild = std::move(*it);

			// Remove the now-null unique_ptr from our collection
			m_children.erase(it);
		}
	}
	
	void SceneNode::DetachFromParent()
	{
		if (m_parent)
		{
			m_parent->RemoveChild(this);
		}
	}
	
	void SceneNode::SetPosition(const Vector3& position)
	{
		m_position       = position;
		m_transformDirty = true;
	}
	
	void SceneNode::SetRotation(const Quaternion& rotation)
	{
		m_rotation       = rotation;
		m_transformDirty = true;
	}
	
	void SceneNode::SetScale(const Vector3& scale)
	{
		m_scale          = scale;
		m_transformDirty = true;
	}
	
	void SceneNode::SetTransform(const Matrix& transform)
	{
		Matrix localTransform = transform;
		localTransform.Decompose(m_position, m_rotation, m_scale);
		m_transformDirty = false;
	}
	
	void SceneNode::Update(const f32 deltaTime)
	{
		for (auto& child : m_children)
		{
			child->Update(deltaTime);
		}
	}
	
	void SceneNode::UpdateTransforms(const Matrix& parentTransform)
	{
		UpdateLocalTransform();

		// Calculate world transform by combining with parent transform
		m_worldTransform = m_localTransform * parentTransform;

		for (auto& child : m_children)
		{
			child->UpdateTransforms(m_worldTransform);
		}
	}
	
	void SceneNode::Render(Gfx::Renderer& renderer, Gfx::Camera& camera) const
	{
		if (!m_visible)
		{
			return;
		}

		// Render this node if it has render data
		if (m_renderData.Mesh)
		{
			// Get the mesh to render
			const Gfx::Mesh& mesh = *m_renderData.Mesh;

			// Note: In a complete implementation, we would update a node-specific WVP constant buffer here
			// For now, we assume the App class has already set up the View and Projection matrices
			// and we just need to modify the World matrix to reflect this node's transform

			// The structure we're modifying is defined in App.h as:
			// struct WVP
			// {
			//     Matrix World;
			//     Matrix View;
			//     Matrix Projection;
			// };

			// Access the constant buffer through proper interfaces in a full implementation
			// For now, assume that the WVP constant buffer has been set up by the App class
			// and we just need to draw the mesh

			// In a complete implementation, you would do something like:
			// WVP wvp;
			// wvp.World = m_worldTransform.Transpose();
			// wvp.View = camera.GetViewMatrix().Transpose();
			// wvp.Projection = camera.GetProjectionMatrix().Transpose();
			// renderer.UpdateConstantBuffer(*nodeWvpBuffer, wvp);

			// For this simplified version, just draw the mesh
			renderer.DrawMesh(mesh);
		}

		// Render all children
		for (const auto& child : m_children)
		{
			child->Render(renderer, camera);
		}
	}
	
	void SceneNode::UpdateLocalTransform()
	{
	}
}