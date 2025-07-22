#include "Application.h"

#include <exception>
#include <fstream>
#include <string>
#include <filesystem>

std::string Timestamp()
{
	static std::string dateFormat = "%Y-%m-%d %H:%M:%S";
	static std::string noDateFormat = "%H:%M:%S";

	const std::chrono::time_point now = std::chrono::system_clock::now();
	const std::time_t time = std::chrono::system_clock::to_time_t(now);

	struct tm timeInfo {};
	[[maybe_unused]] const int error = localtime_s(&timeInfo, &time);

	char buffer[20]{};
	[[maybe_unused]] const size_t wcsTimeErr = strftime(buffer, 20, dateFormat.c_str(), &timeInfo);

	return buffer;
}

int main()
{
	Application application("Boids");

	try
	{
		application.Run();
	}
	catch (std::exception& e)
	{
		std::wstring mbMessage = L"Boids crashed... sorry about that.";
		MessageBoxW(NULL, mbMessage.c_str(), L"ERROR!", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

		std::ofstream crashFile;
		crashFile.open("crash.txt", std::ios::out | std::ios::trunc);

		if (crashFile.is_open())
		{
			crashFile << "[" << Timestamp() << "] " << e.what() << '\n';
		}

		crashFile.close();
	}

	return 0;
}