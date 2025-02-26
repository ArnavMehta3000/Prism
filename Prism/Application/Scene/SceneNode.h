#pragma once
#include "StandardTypes.h"
#include "Math/Math.h"
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Common/String.h>
#include <memory>
#include <vector>

namespace Prism
{
	namespace Gfx
	{
		class Mesh;
		class Renderer;
		class Camera;
	}

	class SceneNode
	{
	public:
		struct RenderData
		{
			std::shared_ptr<Gfx::Mesh> Mesh;
		};

	public:
		SceneNode(const Elos::String& name);
		virtual ~SceneNode();


		void AddChild(std::unique_ptr<SceneNode> child);
		void RemoveChild(SceneNode* child);
		void DetachFromParent();

		void SetName(const Elos::String& name) { m_name = name; }
		void SetVisible(bool visible) { m_visible = visible; }
		void SetPosition(const Vector3& position);
		void SetRotation(const Quaternion& rotation);
		void SetScale(const Vector3& scale);
		void SetTransform(const Matrix& transform);
		void SetRenderData(const RenderData& renderData) { m_renderData = renderData; }

		NODISCARD inline const Elos::String& GetName() const { return m_name; }
		NODISCARD inline bool IsVisible() const { return m_visible; }
		NODISCARD inline const Vector3& GetPosition() const { return m_position; }
		NODISCARD inline const Quaternion& GetRotation() const { return m_rotation; }
		NODISCARD inline const Vector3& GetScale() const { return m_scale; }
		NODISCARD inline const Matrix& GetLocalTransform() const { return m_localTransform; }
		NODISCARD inline const Matrix& GetWorldTransform() const { return m_worldTransform; }
		NODISCARD inline const RenderData& GetRenderData() const { return m_renderData; }
		NODISCARD inline const std::vector<std::unique_ptr<SceneNode>>& GetChildren() const { return m_children; }
		NODISCARD inline SceneNode* GetParent() const { return m_parent; }

		virtual void Update(const f32 deltaTime);
		void UpdateTransforms(const Matrix& parentTransform = Matrix::Identity);
		void Render(Gfx::Renderer& renderer, Gfx::Camera& camera) const;

	private:
		void UpdateLocalTransform();

	private:
		Elos::String                            m_name;
		Vector3                                 m_position       = Vector3::Zero;
		Quaternion                              m_rotation       = Quaternion::Identity;
		Vector3                                 m_scale          = Vector3::One;
		Matrix                                  m_localTransform = Matrix::Identity;
		Matrix                                  m_worldTransform = Matrix::Identity;
		SceneNode*                              m_parent         = nullptr;
		std::vector<std::unique_ptr<SceneNode>> m_children;
		RenderData                              m_renderData;
		bool                                    m_transformDirty = true;
		bool                                    m_visible        = true;
	};
}