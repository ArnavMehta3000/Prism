#include "Scene.h"
#include "Utils/Log.h"
#include "Application/Scene/SceneNode.h"
#include "Application/Scene/NodeRenderingSystem.h"
#include "Graphics/Renderer.h"

namespace Prism
{
    Scene::Scene(const Elos::String& name)
        : m_name(name)
    {
        m_rootNode = std::make_unique<SceneNode>("Root");
        m_nodeRegistry["Root"] = m_rootNode.get();
        Log::Info("Created Scene: {}", name);
    }

    Scene::~Scene()
    {
        Log::Info("Destroying Scene: {}", m_name);
        m_nodeRegistry.clear();
        m_meshes.clear();
        m_rootNode.reset();
    }

    void Scene::Update(f32 deltaTime)
    {
        m_rootNode->Update(deltaTime);
        m_rootNode->UpdateTransforms();
    }

    void Scene::Render(MAYBE_UNUSED Gfx::Renderer& renderer, Gfx::Camera& camera)
    {
        if (!m_renderingSystem)
        {
            Log::Warn("Scene '{}' has no rendering system!", m_name);
            return;
        }

        const std::function<void(const SceneNode*)> RenderNode = [&](const SceneNode* node)
        {
            if (!node || !node->IsVisible())
            {
                return;
            }

            // Render this node
            if (node->GetRenderData().Mesh)
            {
                m_renderingSystem->RenderNode(*node, camera);
            }

            // Render all children
            for (const auto& child : node->GetChildren())
            {
                RenderNode(child.get());
            }
        };

        RenderNode(m_rootNode.get());
    }

    bool Scene::Init(Gfx::Renderer& renderer)
    {
        m_renderingSystem = std::make_unique<NodeRenderingSystem>(renderer);
        if (!m_renderingSystem->Init())
        {
            Log::Error("Failed to initialize rendering system for scene '{}'", m_name);
            return false;
        }

        Log::Info("Scene '{}' initialized successfully", m_name);
        return true;
    }

    SceneNode* Scene::FindNode(const std::string& name) const
    {
        auto it = m_nodeRegistry.find(name);
        return (it != m_nodeRegistry.end()) ? it->second : nullptr;
    }

    SceneNode* Scene::CreateNode(const std::string& name, SceneNode* parent)
    {
        if (m_nodeRegistry.find(name) != m_nodeRegistry.end())
        {
            Log::Warn("Node with name '{}' already exists in the scene.", name);
            return nullptr;
        }

        auto node = std::make_unique<SceneNode>(name);
        SceneNode* nodePtr = node.get();

        m_nodeRegistry[name] = nodePtr;

        if (parent)
        {
            parent->AddChild(std::move(node));
        }
        else
        {
            m_rootNode->AddChild(std::move(node));
        }

        return nodePtr;
    }

    std::shared_ptr<Gfx::Mesh> Scene::GetMesh(const std::string& name) const
    {
        auto it = m_meshes.find(name);
        return (it != m_meshes.end()) ? it->second : nullptr;
    }

    void Scene::AddMesh(const std::string& name, std::shared_ptr<Gfx::Mesh> mesh)
    {
        if (!mesh)
        {
            Log::Warn("Attempted to add null mesh to scene with name '{}'", name);
            return;
        }

        m_meshes[name] = mesh;
    }
    
    void Scene::SetVertexShader(Gfx::Shader* shader)
    {
        if (m_renderingSystem)
        {
            m_renderingSystem->SetVertexShader(shader);
        }
    }
    
    void Scene::SetPixelShader(Gfx::Shader* shader)
    {
        if (m_renderingSystem)
        {
            m_renderingSystem->SetPixelShader(shader);
        }
    }
}
