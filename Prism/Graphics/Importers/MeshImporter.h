#pragma once
#include "StandardTypes.h"
#include "Graphics/Mesh.h"
#include <Elos/Common/String.h>
#include <Elos/Common/FunctionMacros.h>
#include <filesystem>
#include <expected>
#include <vector>
#include <memory>

namespace fs = std::filesystem;

namespace Prism::Gfx
{
    class ResourceFactory;

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
            };

            Type Type;
            HRESULT ErrorCode;
            Elos::String Message;
        };

        struct MeshData
        {
            std::vector<std::shared_ptr<Mesh>> Meshes;
        };

        struct ImportSettings
        {
            bool FlipUVs               = true;
            bool CalculateNormals      = true;
            bool CalculateTangents     = true;
            bool Triangulate           = true;
            bool JoinIdenticalVertices = true;
            bool ConvertToLeftHanded   = true;
            bool FlipWindingOrder      = false;
            bool OptimizeMeshes        = true;
            bool Validate              = true;
        };

    public:
        static NODISCARD std::expected<MeshData, ImportError> Import(
            const ResourceFactory& resourceFactory,
            const fs::path& filePath,
            const ImportSettings& settings = {});

    private:
        static u32 GetAssimpImportFlags(const ImportSettings& settings);
    };
}