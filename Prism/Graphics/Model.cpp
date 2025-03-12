#include "Graphics/Model.h"
#include "Graphics/Renderer.h"
#include "Graphics/Utils/ResourceFactory.h"

namespace Prism::Gfx
{
	Model::Model(std::vector<std::shared_ptr<Mesh>> meshes)
		: m_meshes(std::move(meshes))
	{
	}
	
	void Model::AddMesh(std::shared_ptr<Mesh> mesh)
	{
		if (mesh)
		{
			m_meshes.push_back(std::move(mesh));
		}
	}
	
	void Model::Render(const Renderer& renderer) const
	{
		for (const auto& mesh : m_meshes)
		{
			if (mesh) LIKELY
			{
				mesh->Render(renderer);
			}
		}
	}
	
	std::expected<std::shared_ptr<Model>, MeshImporter::ImportError>
		Model::LoadFromFile(const ResourceFactory& resourceFactory, const fs::path& filePath,
			const MeshImporter::ImportSettings& settings)
	{
		auto importResult = MeshImporter::Import(resourceFactory, filePath, settings);
		if (!importResult)
		{
			return std::unexpected(importResult.error());
		}

		return std::make_shared<Model>(importResult.value().Meshes);
	}
}