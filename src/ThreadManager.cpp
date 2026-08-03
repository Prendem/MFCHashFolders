#include "ThreadManager.hpp"

void ThreadManager::executeThread()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(m_taskQueueMutex);
		m_conditionVariable.wait(lock, [&](){ return !m_taskQueue.empty() || m_stop; });

		if (m_stop)
			return;

		std::function<void()> task{ std::move(m_taskQueue.front()) };
		m_taskQueue.pop();
		lock.unlock();
		task();
	}
}

ThreadManager::ThreadManager()
{
	unsigned int logicalProcessorCount{ std::thread::hardware_concurrency() };
	m_threadPool.reserve(logicalProcessorCount);
	for (unsigned int index{ 0 }; index < logicalProcessorCount; ++index)
	{
		m_threadPool.emplace_back(std::thread(&ThreadManager::executeThread, this));
	}
}

ThreadManager::~ThreadManager()
{
	m_stop = true;
	m_conditionVariable.notify_all();
	for (auto& thread : m_threadPool)
		thread.join();
}

void ThreadManager::enqueueTask(std::packaged_task<std::wstring()>&& inTask, HWND window, size_t index, uint16_t generation, HashType hashType)
{
	auto sharedTask{ std::make_shared<std::packaged_task<std::wstring()>>(std::move(inTask))};
	auto wrappedTask{ [sharedTask, window, index, generation, hashType]()
		{
			(*sharedTask)();
			::PostMessage(window, WM_HASH_COMPLETE, static_cast<WPARAM>(index), MAKELPARAM(generation, static_cast<int>(hashType)));
		} };

	std::lock_guard<std::mutex> lock(m_taskQueueMutex);
	m_taskQueue.emplace(std::move(wrappedTask));
	m_conditionVariable.notify_one();
}

void ThreadManager::purgeQueue()
{
	std::queue<std::function<void()>>().swap(m_taskQueue);
}
