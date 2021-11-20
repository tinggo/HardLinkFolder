#pragma once

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>


class WorkerSchedule
{
public:

	template<typename F>
	WorkerSchedule(int workerCount, F entry)
	{
		for (int i = 0; i < workerCount; ++i)
		{
			std::thread t(&WorkerSchedule::WorkerEntry<F>, this, entry);
			m_threadPool.push_back(std::move(t));
		}
	}

	void Start()
	{
		m_starting.store(true, std::memory_order_relaxed);
	}

	void Join()
	{
		for (std::thread& t : m_threadPool)
		{
			t.join();
		}
	}

private:

	template<typename F>
	void WorkerEntry(F entry)
	{
		while (!m_starting.load(std::memory_order_relaxed))
		{
			std::chrono::milliseconds ms{ 1 };
			std::this_thread::sleep_for(ms);
		}
		for (;;)
		{
			if (!entry())
			{
				break;
			}
		}
	}

	std::vector<std::thread> m_threadPool;
	std::atomic_bool m_starting;
	std::mutex m_poolLock;
};
