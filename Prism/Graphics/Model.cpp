#include "Graphics/Model.h"
#include "Graphics/Renderer.h"
#include "Graphics/Utils/ResourceFactory.h"
#include "Application/Globals.h"

namespace Prism::Gfx
{
	Model::Model(const MeshImporter::MeshData& meshData)
		: m_meshes(meshData.Meshes)
		, m_textures(meshData.Textures)
		, m_textureMap(meshData.TextureMap)
	{		
	}
	
	Model::~Model()
	{
		m_meshes.clear();
		m_textures.clear();
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
			if (!m_textures.empty())
			{
				auto& texture = m_textures[Globals::g_textureNumber];
				ID3D11ShaderResourceView* srvs[] = { texture->GetSRV() };
				renderer.SetShaderResourceViews(Shader::Type::Pixel, 0, std::span{ srvs });
			}

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
		return std::make_shared<Model>(importResult.value());;
	}
}