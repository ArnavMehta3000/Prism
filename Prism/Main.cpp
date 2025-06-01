#include "Utils/Log.h"
#include "Application/App.h"
#include <filesystem>

int main()
{
	try
	{
		Prism::Log::Init();

		Prism::Log::Warn("Current working directory: {}", std::filesystem::current_path().string());

		Prism::App app;
		app.Run();
	}
	catch (const std::exception& e)
	{
		Prism::Log::Error("Exception thrown: {}", e.what());
	}

	return 0;
}
