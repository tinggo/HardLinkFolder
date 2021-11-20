#include "worker.h"
#include <iostream>
#include <chrono>


extern void SetHardLinkPath(const std::wstring& srcPath, const std::wstring& desPath);
extern bool Explore();


void Instruction()
{
	std::cout << "Format: HardLinkFolder <source folder path> <linkage path>" << std::endl;
}


int wmain(int argc, wchar_t* argv[])
{
	if (argc != 3)
	{
		Instruction();
		return -1;
	}

	SetHardLinkPath(argv[1], argv[2]);

	int workerCount = std::thread::hardware_concurrency();
	if (!workerCount)
	{
		workerCount = 4;
	}
	WorkerSchedule ws(workerCount, Explore);

	auto beginTP = std::chrono::steady_clock::now();

	ws.Start();
	ws.Join();

	auto d = std::chrono::steady_clock::now() - beginTP;
	auto s = std::chrono::duration_cast<std::chrono::seconds>(d);
	std::cout << std::endl << "Time cost: " << s.count() << "s" <<std::endl;

	return 0;
}
