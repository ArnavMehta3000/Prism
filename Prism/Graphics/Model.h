#pragma once
#include "Application/CommonTypes.h"
#include "Graphics/Importers/MeshImporter.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace Prism::Gfx
{
	class Renderer;
	class ResourceFactory;

	class Model
	{
	public:
		Model() = default;
		~Model() = default;
		explicit Model(std::vector<std::shared_ptr<Mesh>> meshes);

		NODISCARD inline Transform& GetTransform() { return m_transform; }
		
		void AddMesh(std::shared_ptr<Mesh> mesh);
		void Render(const Renderer& renderer) const;

		static std::expected<std::shared_ptr<Model>, MeshImporter::ImportError> LoadFromFile(
			const ResourceFactory& resourceFactory, const fs::path& filePath, const MeshImporter::ImportSettings& settings);

	private:
		Transform m_transform;
		std::vector<std::shared_ptr<Mesh>> m_meshes;
	};
}