#pragma once
#include "StandardTypes.h"
#include "Graphics/Mesh.h"
#include <Elos/Common/String.h>
#include <Elos/Common/FunctionMacros.h>
#include <filesystem>
#include <expected>
#include <vector>
#include <unordered_map>

namespace fs = std::filesystem;
struct aiScene;
struct aiNode;
struct aiMesh;
struct aiTexture;

namespace Prism::Gfx
{
    class ResourceFactory;
    class Texture2D;

    class MeshImporter
    {
    public:
        struct ImportError
        {
            enum class Type
            {
                FileNotFound,
                AssimpError,
                MeshCreationFailed,
                NoMeshesFound,
                TextureLoadingFailed
            };

            Type Type;
            HRESULT ErrorCode;
            Elos::String Message;
        };

        struct MeshData
        {
            std::vector<std::shared_ptr<Mesh>> Meshes;
            std::vector<std::shared_ptr<Texture2D>> Textures;
            std::unordered_map<Elos::String, u64> TextureMap;
        };

        struct ImportSettings
        {
            bool FlipUVs                 = true;
            bool CalculateNormals        = true;
            bool CalculateTangents       = true;
            bool Triangulate             = true;
            bool JoinIdenticalVertices   = true;
            bool ConvertToLeftHanded     = true;
            bool FlipWindingOrder        = false;
            bool OptimizeMeshes          = true;
            bool Validate                = true;
            bool ExtractEmbeddedTextures = true;
        };

    public:
        static NODISCARD std::expected<MeshData, ImportError> Import(
            const ResourceFactory& resourceFactory,
            const fs::path& filePath,
            const ImportSettings& settings = {});


    private:
        static std::expected<void, ImportError> ProcessNode(const ResourceFactory& resourceFactory, MeshData& meshData, aiNode* node, const aiScene* scene);
        static std::expected<void, ImportError> ProcessMesh(const ResourceFactory& resourceFactory, MeshData& meshData, aiMesh* mesh, const aiScene* scene);
        static std::expected<void, ImportError> LoadTextures(const ResourceFactory& resourceFactory, MeshData& meshData, const aiScene* scene);
        static std::expected<std::shared_ptr<Texture2D>, Texture2D::TextureError> CreateTextureFromData(
            const ResourceFactory& resourceFactory,
            const std::vector<byte>& textureData,
            const aiTexture* aiTex);
        static std::expected<std::shared_ptr<Texture2D>, Texture2D::TextureError> CreateTextureFromCompressedData(
            const ResourceFactory& resourceFactory,
            const std::vector<byte>& compressedData);
        static u32 GetAssimpImportFlags(const ImportSettings& settings);
    };
}