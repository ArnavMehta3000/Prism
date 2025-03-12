#include "Graphics/Importers/MeshImporter.h"
#include "Graphics/Mesh.h"
#include "Graphics/Utils/ResourceFactory.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <VertexTypes.h>

namespace Prism::Gfx
{	
	std::expected<MeshImporter::MeshData, MeshImporter::ImportError> MeshImporter::Import(
		const ResourceFactory& resourceFactory, const fs::path& filePath, const ImportSettings& settings)
	{
		if (!fs::exists(filePath))
		{
			return std::unexpected(ImportError
			{
				.Type      = ImportError::Type::FileNotFound,
				.ErrorCode = E_FAIL,
				.Message   = "File not found: " + filePath.string()
			});
		}

		Assimp::Importer importer;
		const u32 flags = GetAssimpImportFlags(settings);
		
		const aiScene* scene = importer.ReadFile(filePath.string(), flags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			return std::unexpected(ImportError
			{
				.Type      = ImportError::Type::AssimpError,
				.ErrorCode = E_FAIL,
				.Message   = "Assimp error: " + Elos::String(importer.GetErrorString())
			});
		}

		MeshData meshData;
		meshData.Meshes.reserve(scene->mNumMeshes);

		using VertexType = DirectX::VertexPositionNormalTangentColorTexture;

		for (u32 i = 0; i < scene->mNumMeshes; i++)
		{
			const aiMesh* mesh = scene->mMeshes[i];

			std::vector<VertexType> vertices;
			std::vector<u32> indices;

			vertices.reserve(mesh->mNumVertices);
			indices.reserve(mesh->mNumFaces * 3);

			// Extract vertices
			for (u32 j = 0; j < mesh->mNumVertices; j++)
			{
				VertexType vertex{};

				vertex.position.x = mesh->mVertices[j].x;
				vertex.position.y = mesh->mVertices[j].y;
				vertex.position.z = mesh->mVertices[j].z;

				if (mesh->HasNormals())
				{
					vertex.normal.x = mesh->mNormals[j].x;
					vertex.normal.y = mesh->mNormals[j].y;
					vertex.normal.z = mesh->mNormals[j].z;
				}

				if (mesh->HasTextureCoords(0))
				{
					vertex.textureCoordinate.x = mesh->mTextureCoords[0][j].x;
					vertex.textureCoordinate.y = mesh->mTextureCoords[0][j].y;
				}

				if (mesh->HasTangentsAndBitangents())
				{
					vertex.tangent.x = mesh->mTangents[j].x;
					vertex.tangent.y = mesh->mTangents[j].y;
					vertex.tangent.z = mesh->mTangents[j].z;
				}

				vertices.push_back(vertex);
			}

			// Extract indices
			for (u32 j = 0; j < mesh->mNumFaces; j++)
			{
				const aiFace& face = mesh->mFaces[j];
				for (u32 k = 0; k < face.mNumIndices; ++k)
				{
					indices.push_back(face.mIndices[k]);
				}
			}

			Mesh::MeshDesc meshDesc;
			meshDesc.VertexStride = sizeof(VertexType);
			meshDesc.Topology     = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			auto meshResult = resourceFactory.CreateMesh(
				vertices.data(),
				static_cast<u32>(vertices.size()),
				std::span(indices),
				meshDesc);

			if (!meshResult)
			{
				return std::unexpected(ImportError
				{
					.Type      = ImportError::Type::MeshCreationFailed,
					.ErrorCode = meshResult.error().ErrorCode,
					.Message   = "Failed to create mesh: " + meshResult.error().Message
				});
			}

			meshData.Meshes.push_back(std::move(meshResult.value()));
		}

		return meshData;
	}
	
	u32 MeshImporter::GetAssimpImportFlags(const ImportSettings& settings)
	{
		u32 flags = 0;

		if (settings.Triangulate) 
		{
			flags |= aiProcess_Triangulate;
		}

		if (settings.JoinIdenticalVertices)
		{
			flags |= aiProcess_JoinIdenticalVertices;
		}

		if (settings.CalculateNormals)
		{
			flags |= aiProcess_GenSmoothNormals;
		}

		if (settings.CalculateTangents)
		{
			flags |= aiProcess_CalcTangentSpace;
		}

		if (settings.ConvertToLeftHanded)
		{
			flags |= aiProcess_MakeLeftHanded;
		}

		if (settings.FlipWindingOrder)
		{
			flags |= aiProcess_FlipWindingOrder;
		}

		if (settings.OptimizeMeshes)
		{
			flags |= aiProcess_OptimizeMeshes;
		}

		if (settings.FlipUVs)
		{
			flags |= aiProcess_FlipUVs;
		}

		if (settings.Validate)
		{
			flags |= aiProcess_ValidateDataStructure;
		}

		return flags;
	}
}