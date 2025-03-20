#pragma once
#include <print>
#include <chrono>
#include <ctime>

namespace Prism
{
	class Log
	{
	public:
		static void Init();

		template <typename... Args>
		static inline void Info(std::format_string<Args...> fmt, Args&&... args)
		{
			PrintLog(WHITE, "INFO", fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		static inline void Warn(std::format_string<Args...> fmt, Args&&... args)
		{
			PrintLog(YELLOW, "WARN", fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		static inline void Error(std::format_string<Args...> fmt, Args&&... args)
		{
			PrintLog(RED, "ERROR", fmt, std::forward<Args>(args)...);
		}

	private:
		template<typename... Args>
		static void PrintLog(const char* color, std::string_view level, std::format_string<Args...> fmt, Args&&... args)
		{
			auto now = std::chrono::system_clock::now();
			auto time_t_now = std::chrono::system_clock::to_time_t(now);
			std::tm tm_now{};
			localtime_s(&tm_now, &time_t_now);

			auto time_str = std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
				tm_now.tm_year + 1900,
				tm_now.tm_mon + 1,
				tm_now.tm_mday,
				tm_now.tm_hour,
				tm_now.tm_min,
				tm_now.tm_sec);

			std::println("{}{} [{}] [{}] {}{}",
				color,
				BOLD,
				time_str,
				level,
				std::format(fmt, std::forward<Args>(args)...),
				RESET);
		}

	private:
		static constexpr const char* RESET   = "\033[0m";
		static constexpr const char* RED     = "\033[31m";
		static constexpr const char* GREEN   = "\033[32m";
		static constexpr const char* YELLOW  = "\033[33m";
		static constexpr const char* BLUE    = "\033[34m";
		static constexpr const char* MAGENTA = "\033[35m";
		static constexpr const char* CYAN    = "\033[36m";
		static constexpr const char* WHITE   = "\033[37m";
		static constexpr const char* BOLD    = "\033[1m";
	};
}
