#include "Graphics/Importers/MeshImporter.h"
#include "Graphics/Mesh.h"
#include "Graphics/Utils/ResourceFactory.h"
#include "Utils/Log.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>


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

		if (auto result = ProcessNode(resourceFactory, meshData, scene->mRootNode, scene); !result)
		{
			return std::unexpected(result.error());
		}

		if (auto result = LoadTextures(resourceFactory, meshData, scene); !result)
		{
			// We don't exit if we fail to import textures
			auto& error = result.error();
			Log::Warn("Failed to import texture {} for mesh or model {}",
				error.Message, filePath.string());
		}

		return meshData;
	}
	
	std::expected<void, MeshImporter::ImportError> MeshImporter::ProcessNode(const ResourceFactory& resourceFactory, MeshData& meshData, aiNode* node, const aiScene* scene)
	{
		// Process meshes for this node
		for (u32 i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			if (auto result = ProcessMesh(resourceFactory, meshData, mesh, scene); !result)
			{
				return std::unexpected(result.error());
			}
		}

		// Process children node
		for (UINT i = 0; i < node->mNumChildren; i++)
		{
			if (auto result = ProcessNode(resourceFactory, meshData, node->mChildren[i], scene); !result)
			{
				return std::unexpected(result.error());
			}
		}

		return {};
	}

	std::expected<void, MeshImporter::ImportError> MeshImporter::ProcessMesh(const ResourceFactory& resourceFactory, MeshData& meshData, aiMesh* mesh, [[maybe_unused]] const aiScene* scene)
	{
		using VertexType = DirectX::VertexPositionNormalTangentColorTexture;

		std::vector<VertexType> vertices;
		std::vector<u32> indices;

		vertices.reserve(mesh->mNumVertices);
		indices.reserve(mesh->mNumFaces * 3);

		// Extract vertices
		for (u32 i = 0; i < mesh->mNumVertices; i++)
		{
			VertexType vertex{};

			vertex.position.x = mesh->mVertices[i].x;
			vertex.position.y = mesh->mVertices[i].y;
			vertex.position.z = mesh->mVertices[i].z;

			if (mesh->HasNormals())
			{
				vertex.normal.x = mesh->mNormals[i].x;
				vertex.normal.y = mesh->mNormals[i].y;
				vertex.normal.z = mesh->mNormals[i].z;
			}

			if (mesh->HasTextureCoords(0))
			{
				vertex.textureCoordinate.x = mesh->mTextureCoords[0][i].x;
				vertex.textureCoordinate.y = mesh->mTextureCoords[0][i].y;
			}

			if (mesh->HasTangentsAndBitangents())
			{
				vertex.tangent.x = mesh->mTangents[i].x;
				vertex.tangent.y = mesh->mTangents[i].y;
				vertex.tangent.z = mesh->mTangents[i].z;
			}

			vertices.push_back(vertex);
		}

		// Extract indices
		for (u32 i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			for (u32 j = 0; j < face.mNumIndices; ++j)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		Mesh::MeshDesc meshDesc;
		meshDesc.VertexStride = sizeof(VertexType);
		meshDesc.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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
		
		return {};
	}

	std::expected<void, MeshImporter::ImportError> MeshImporter::LoadTextures(const ResourceFactory& resourceFactory, MeshData& meshData, const aiScene* scene)
	{
		if (!scene->HasTextures())
		{
			return {};
		}

		meshData.Textures.reserve(scene->mNumTextures);

		for (u32 i = 0; i < scene->mNumTextures; i++)
		{
			const aiTexture* texture = scene->mTextures[i];
			
			std::vector<byte> textureData;

			if (texture->mHeight == 0)
			{
				// Compressed texture: data is stored as a continuous block
				textureData.resize(texture->mWidth);
				std::memcpy(textureData.data(), texture->pcData, texture->mWidth);
			}
			else
			{
				// Uncompressed texture: data is stored as an array of texels
				const size_t dataSize = texture->mWidth * texture->mHeight * 4; // RGBA
				textureData.resize(dataSize);

				for (u32 h = 0; h < texture->mHeight; h++)
				{
					for (u32 w = 0; w < texture->mWidth; w++)
					{
						const aiTexel& texel = texture->pcData[h * texture->mWidth + w];
						const size_t index = (h * texture->mWidth + w) * 4;

						textureData[index + 0] = static_cast<byte>(texel.r);
						textureData[index + 1] = static_cast<byte>(texel.g);
						textureData[index + 2] = static_cast<byte>(texel.b);
						textureData[index + 3] = static_cast<byte>(texel.a);
					}
				}
			}

			auto textureResult = CreateTextureFromData(resourceFactory, textureData, texture);
			if (!textureResult)
			{
				return std::unexpected(ImportError
				{
					.Type      = ImportError::Type::TextureLoadingFailed,
					.ErrorCode = textureResult.error().ErrorCode,
					.Message   = "Failed to create texture from embedded data: " + textureResult.error().Message
				});
			}

			// Store the texture
			meshData.Textures.push_back(std::move(textureResult.value()));

			// Create a mapping for the texture name/identifier
			const Elos::String textureName = texture->mFilename.length > 0
				? Elos::String(texture->mFilename.C_Str())
				: "EmbeddedTexture_" + std::to_string(i);
			
			meshData.TextureMap[textureName] = i;
			Log::Info("Loaded texture: {}", textureName);
		}


		return {};
	}

	std::expected<std::shared_ptr<Texture2D>, Texture2D::TextureError> 
		MeshImporter::CreateTextureFromData(const ResourceFactory& resourceFactory, const std::vector<byte>& textureData, const aiTexture* aiTex)
	{
		if (aiTex->mHeight == 0)
		{
			// Compressed texture - use WIC to load
			return CreateTextureFromCompressedData(resourceFactory, textureData);
		}
		else
		{
			// Uncompressed RGBA texture
			Texture2D::Texture2DDesc desc;
			desc.Width          = aiTex->mWidth;
			desc.Height         = aiTex->mHeight;
			desc.Format         = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.Usage          = D3D11_USAGE_DEFAULT;
			desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags      = 0;
			desc.MipLevels      = 1;
			desc.ArraySize      = 1;

			const u32 rowPitch = aiTex->mWidth * 4; // 4 bytes per pixel for RGBA
			return resourceFactory.CreateTexture2D(desc, textureData.data(), rowPitch);
		}
	}

	std::expected<std::shared_ptr<Texture2D>, Texture2D::TextureError> 
		MeshImporter::CreateTextureFromCompressedData(const ResourceFactory& resourceFactory, const std::vector<byte>& compressedData)
	{
		return resourceFactory.CreateTextureFromWIC(
			compressedData.data(),
			static_cast<u32>(compressedData.size())
		);
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