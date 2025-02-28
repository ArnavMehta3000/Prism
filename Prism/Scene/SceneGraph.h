#pragma once
#include "StandardTypes.h"
#include <Elos/Common/String.h>
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Interface/Interface.h>
#include <functional>
#include <any>
#include <format>
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <optional>

namespace Prism
{
	class Node;
	class SceneGraph;

	template <typename T>
	concept PropertyType = std::copyable<T> || std::movable<T>;

	ELOS_DECLARE_INTERFACE(IComponentOwner,
		{ t.GetOwner() } -> std::same_as<Node*>;
	);
	
	ELOS_DECLARE_INTERFACE(IUpdatable,
		{ t.Update(std::declval<const f32>()) } -> std::same_as<void>;
	);

	class Component : public Elos::Interface<IComponentOwner, IUpdatable>
	{
	public:
		explicit Component(Node* owner) : m_owner(owner) {}
		virtual ~Component() = default;

		virtual void Update(MAYBE_UNUSED f32 dt) {}
		inline NODISCARD Node* GetOwner() const { return m_owner; }

	private:
		Node* m_owner = nullptr;
	};

	using EventCallback = std::function<void(const std::any&)>;

	class Node
	{
		friend class SceneGraph;

	public:
		explicit Node(std::string name) : m_name(std::move(name)) {}

		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;
		Node(Node&&) noexcept = default;
		Node& operator=(Node&&) noexcept = default;
		virtual ~Node() = default;

		inline NODISCARD const Elos::String& GetName() const { return m_name; }
		inline NODISCARD Node* GetParent() const { return m_parent; }
		inline NODISCARD SceneGraph* GetSceneGraph() const { return m_sceneGraph; }

		template <typename NodeType = Node, typename... Args> requires std::derived_from<NodeType, Node>
		NodeType& CreateChild(const Elos::String& childName, Args&&... args)
		{
			auto child = std::make_unique<NodeType>(std::move(childName), std::forward<Args>(args)...);
			child->m_sceneGraph = m_sceneGraph;
			child->m_parent = this;
			m_children.emplace_back(std::move(child));
			return *static_cast<NodeType*>(m_children.back().get());
		}

		void AddChild(std::unique_ptr<Node> child) 
		{
			child->m_parent = this;
			child->m_sceneGraph = m_sceneGraph;
			m_children.push_back(std::move(child));
		}

		NODISCARD std::unique_ptr<Node> RemoveChild(const Elos::String& name) 
		{
			auto it = std::ranges::find_if(m_children, [&](const auto& child) 
			{
				return child->m_name == name;
			});

			if (it != m_children.end()) 
			{
				std::unique_ptr<Node> child = std::move(*it);
				m_children.erase(it);
				child->m_parent = nullptr;
				child->m_sceneGraph = nullptr;
				return child;
			}

			return nullptr;
		}

		template <typename T, typename... Args> requires std::derived_from<T, Component>
		T& AddComponent(Args&&... args) 
		{
			auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
			auto& ref = *component;
			m_components[typeid(T)] = std::move(component);
			return ref;
		}

		template <typename T>
		NODISCARD T* GetComponent()
		{
			auto it = m_components.find(typeid(T));
			if (it != m_components.end())
			{
				return dynamic_cast<T*>(it->second.get());
			}
			return nullptr;
		}

		template <typename Interface> requires Elos::IsInterface_v<Interface>
		NODISCARD std::vector<Component*> GetComponentsWithInterface()
		{
			std::vector<Component*> result;
			for (auto& [type, component] : m_components)
			{
				if (dynamic_cast<Interface*>(component.get())) 
				{
					result.push_back(component.get());
				}
			}
			return result;
		}

		template <typename T>
		void RemoveComponent()
		{
			m_components.erase(typeid(T));
		}

		template <typename T> requires PropertyType<T>
		void SetProperty(T value) 
		{
			m_properties[typeid(T)] = std::move(value);
		}

		template <typename T>
		NODISCARD std::optional<std::reference_wrapper<T>> GetProperty()
		{
			auto it = m_properties.find(typeid(T));
			if (it != m_properties.end()) 
			{
				try 
				{
					return std::optional<std::reference_wrapper<T>>(std::any_cast<T&>(it->second));
				}
				catch (const std::bad_any_cast&) 
				{
					return std::nullopt;
				}
			}
			return std::nullopt;
		}
		
		template <typename T>
		NODISCARD bool HasProperty() const
		{
			return m_properties.contains(typeid(T));
		}

		void AddEventListener(const std::string& eventType, EventCallback callback)
		{
			m_eventHandlers[eventType].push_back(std::move(callback));
		}

		void TriggerEvent(const std::string& eventType, const std::any& eventData)
		{
			auto it = m_eventHandlers.find(eventType);
			if (it != m_eventHandlers.end()) 
			{
				for (const auto& handler : it->second) 
				{
					handler(eventData);
				}
			}

			// Propagate to parent
			if (m_parent) 
			{
				m_parent->TriggerEvent(eventType, eventData);
			}
		}

		template <typename Func> requires std::invocable<Func, Node&>
		void ForEach(Func&& func)
		{
			// Call on self then propagate
			func(*this);
			for (auto& child : m_children)
			{
				child->ForEach(func);
			}
		}

		template <typename Predicate> requires std::predicate<Predicate, Node&>
		NODISCARD Node* FindChild(Predicate&& predicate)
		{
			for (auto& child : m_children)
			{
				if (predicate(*child))
				{
					return child.get();
				}

				if (auto result = child->FindChild(predicate))
				{
					return result;
				}
			}
			return nullptr;
		}

		template<typename Predicate> requires std::predicate<Predicate, Node&>
		NODISCARD auto FindChildren(Predicate&& predicate)
		{
			std::vector<Node*> result;

			ForEach([&](Node& node)
			{
				if (&node != this && predicate(node))
				{
					result.push_back(&node);
				}
			});

			return result;
		}

		virtual void Update(const f32 deltaTime) 
		{
			// Update all components
			for (auto& [type, component] : m_components)
			{
				component->Update(deltaTime);
			}

			// Update children
			for (auto& child : m_children)
			{
				child->Update(deltaTime);
			}
		}

		NODISCARD std::string GetDebugInfo(int indent = 0) const 
		{
			std::string result = std::string(indent, ' ') + std::format("Node: {} ({})\n", m_name, (void*)this);

			result += std::string(indent + 2, ' ') + "Components:\n";
			for (const auto& [type, component] : m_components)
			{
				result += Elos::String(indent + 4, ' ') + std::format("{}\n", type.name());
			}

			result += std::string(indent + 2, ' ') + "Properties:\n";
			for (const auto& [type, property] : m_properties)
			{
				result += Elos::String(indent + 4, ' ') + std::format("{}\n", type.name());
			}

			result += Elos::String(indent + 2, ' ') + "Children:\n";
			for (const auto& child : m_children)
			{
				result += child->GetDebugInfo(indent + 4);
			}

			return result;
		}

	private:
		Elos::String                                                    m_name;
		Node*                                                           m_parent = nullptr;
		std::vector<std::unique_ptr<Node>>                              m_children;
		std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
		std::unordered_map<std::type_index, std::any>                   m_properties;
		std::unordered_map<Elos::String, std::vector<EventCallback>>    m_eventHandlers;
		SceneGraph*                                                     m_sceneGraph = nullptr;
	};


	class SceneGraph
	{
	public:
		SceneGraph() : m_root(std::make_unique<Node>("Root")) 
		{
			m_root->m_sceneGraph = this;
			RegisterNode(*m_root);
		}

		SceneGraph(const SceneGraph&) = delete;
		SceneGraph& operator=(const SceneGraph&) = delete;
		SceneGraph(SceneGraph&&) noexcept = default;
		SceneGraph& operator=(SceneGraph&&) noexcept = default;
		~SceneGraph() = default;

		NODISCARD inline Node& GetRoot() noexcept { return *m_root; }
		NODISCARD inline const Node& GetRoot() const noexcept { return *m_root; }

		void RegisterNode(Node& node) 
		{
			m_nodeRegistry[node.GetName()] = &node;
		}

		void UnregisterNode(const Elos::String& name)
		{
			m_nodeRegistry.erase(name);
		}

		NODISCARD Node* FindNodeByName(const std::string& name) 
		{
			auto it = m_nodeRegistry.find(name);
			if (it != m_nodeRegistry.end()) 
			{
				return it->second;
			}
			return nullptr;
		}

		template <typename Interface> requires Elos::IsInterface_v<Interface>
		NODISCARD std::vector<Node*> FindNodesWithInterface() 
		{
			std::vector<Node*> result;
			m_root->ForEach([&](Node& node) 
			{
				if (dynamic_cast<Interface*>(&node))
				{
					result.push_back(&node);
				}
			});
			return result;
		}

		void Update(float deltaTime) 
		{
			m_root->Update(deltaTime);
		}

	private:
		std::unique_ptr<Node> m_root;
		std::unordered_map<Elos::String, Node*> m_nodeRegistry;
	};
}