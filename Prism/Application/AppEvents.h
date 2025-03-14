#pragma once
#include <Elos/Event/Signal.h>
#include <Elos/Window/Window.h>

namespace Prism
{
	struct AppEvents
	{
		Elos::Signal<const Elos::Event::Resized&>             OnResized;
		Elos::Signal<const Elos::Event::KeyPressed&>          OnKeyPressed;
		Elos::Signal<const Elos::Event::KeyReleased&>         OnKeyReleased;
		Elos::Signal<const Elos::Event::MouseMovedRaw&>       OnMouseMovedRaw;
		Elos::Signal<const Elos::Event::MouseButtonPressed&>  OnMouseButtonPressed;
		Elos::Signal<const Elos::Event::MouseButtonReleased&> OnMouseButtonReleased;
		Elos::Signal<const Elos::Event::MouseWheelScrolled&>  OnMouseWheelScrolled;
	};

	struct AppEventConnections
	{
		Elos::Connection<const Elos::Event::Resized&>             OnResized;
		Elos::Connection<const Elos::Event::KeyPressed&>          OnKeyPressed;
		Elos::Connection<const Elos::Event::KeyReleased&>         OnKeyReleased;
		Elos::Connection<const Elos::Event::MouseMovedRaw&>       OnMouseMovedRaw;
		Elos::Connection<const Elos::Event::MouseButtonPressed&>  OnMouseButtonPressed;
		Elos::Connection<const Elos::Event::MouseButtonReleased&> OnMouseButtonReleased;
		Elos::Connection<const Elos::Event::MouseWheelScrolled&>  OnMouseWheelScrolled;

		void DisconnectAll()
		{
			OnResized.Disconnect();
			OnKeyPressed.Disconnect();
			OnKeyReleased.Disconnect();
			OnMouseMovedRaw.Disconnect();
			OnMouseButtonPressed.Disconnect();
			OnMouseButtonReleased.Disconnect();
			OnMouseWheelScrolled.Disconnect();
		}
	};
}