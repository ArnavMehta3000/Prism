#include "Utils/Log.h"
#include "Application/App.h"

int main()
{
	try
	{
		Prism::Log::Init();
		Prism::App app;
		app.Run();
	}
	catch (const std::exception& e)
	{
		Prism::Log::Error("Exception thrown: {}", e.what());
	}

	return 0;
}
