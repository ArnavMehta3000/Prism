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
		~Model();
		explicit Model(const MeshImporter::MeshData& meshData);

		NODISCARD inline Transform& GetTransform() { return m_transform; }
		NODISCARD inline auto& GetTextures() { return m_textures; }
		
		void AddMesh(std::shared_ptr<Mesh> mesh);
		void Render(const Renderer& renderer) const;

		static std::expected<std::shared_ptr<Model>, MeshImporter::ImportError> LoadFromFile(
			const ResourceFactory& resourceFactory, const fs::path& filePath, const MeshImporter::ImportSettings& settings);

	private:
		Transform                               m_transform;
		std::vector<std::shared_ptr<Mesh>>      m_meshes;
		std::vector<std::shared_ptr<Texture2D>> m_textures;
		std::unordered_map<Elos::String, u64>   m_textureMap;
	};
}