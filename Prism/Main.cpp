#include "Utils/Log.h"
#include "Application/App.h"

int main()
{
	try
	{
		Px::App app;
		app.Run();
	}
	catch (const std::exception& e)
	{
		Px::Log::Error("Exception thrown: {}", e.what());
	}

	return 0;
}	