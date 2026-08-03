#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

#include "FileUtil.hpp"

class ThreadManager
{
private:
	std::vector<std::thread> m_threadPool;
	std::queue<std::function<void()>> m_taskQueue;
	std::mutex m_taskQueueMutex;
	std::condition_variable m_conditionVariable;
	bool m_stop = false;

	void executeThread();

public:
	ThreadManager();
	~ThreadManager();
	void enqueueTask(std::packaged_task<std::wstring()>&& inTask, HWND window, size_t index, uint16_t generation, HashType hashType);
	void purgeQueue();
};