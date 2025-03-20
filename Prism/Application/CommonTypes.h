#pragma once
#include "Math/Math.h"

namespace Prism
{
	struct WVP
	{
		Matrix World;
		Matrix View;
		Matrix Projection;
	};

	struct Transform
	{
		Vector3 Position = Vector3::Zero;
		Vector3 Rotation = Vector3::Zero;
		Vector3 Scale    = Vector3::One;

		Transform()
		{
			UpdateWorldMatrix();
		}

		void UpdateWorldMatrix()
		{
			m_worldMatrix = Matrix::CreateScale(Scale)
				* Matrix::CreateFromYawPitchRoll(Rotation)
				* Matrix::CreateTranslation(Position);
		}

		inline Matrix GetWorldMatrix() const { return m_worldMatrix; }
		inline Matrix GetTransposedWorldMatrix() const { return m_worldMatrix.Transpose(); }

	private:
		Matrix m_worldMatrix;
	};
}