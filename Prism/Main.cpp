#include "Utils/Log.h"

int main()
{
	Px::Log{};  // Inits console


	Px::Log::Info("Hello, {}!", "World");
	Px::Log::Warn("Low disk space: {}%", 10);
	Px::Log::Error("File not found: {}", "config.txt");

	return 0;
}	