/*
** EPITECH PROJECT, 2026
** Pixel
** File description:
** Pixel def
*/

#ifndef POOL_THREAD
    #define POOL_THREAD

    #include <atomic>
    #include <vector>
    #include <iostream>
    #include <queue>
    #include <mutex>
    #include <functional>
    #include <semaphore>
    #include <thread>
    #include <condition_variable>

/** @class PoolThread 
 * @brief A thread pool for managing and executing tasks concurrently
 * This class implements a thread pool that allows for the concurrent execution of tasks. It manages a queue of tasks and a pool of worker threads that continuously check for and execute tasks from the queue. The thread pool can be used to efficiently manage and execute multiple tasks in parallel, improving performance in scenarios where tasks can be executed concurrently, such as rendering pixels in a ray tracing application.
 * The PoolThread class provides methods for adding tasks to the queue and ensures proper synchronization between threads when accessing the task queue. It also handles graceful shutdown of the worker threads when the pool is destroyed.
 */
class PoolThread {
    private:
        std::vector<std::thread> _threads;
        std::queue<std::function<void()>> _tasks;
        std::mutex _taskAccess;
        std::condition_variable _taskCounter;
        bool _stop;
        std::atomic<int> _activeWorkers;

    public:
        /** @brief Construct a thread pool and start worker threads
         * This constructor initializes the thread pool by creating a number of worker threads equal to the hardware concurrency of the system. Each worker thread runs a loop that continuously checks for and executes tasks from the task queue. The constructor also initializes the counting semaphore to manage access to the task queue and sets the stop flag to false.
         */
        PoolThread(int workers) : _activeWorkers(0) {
            _stop = false;
            for (int i = 0; i < workers; i++) {
                _threads.emplace_back([this](){this->workLoop();});
            }
        }

        /** @brief Destructor for the thread pool
         * This destructor ensures that all worker threads are properly shut down when the thread pool is destroyed. It sets the stop flag to true, releases the semaphore to unblock any waiting threads, and then joins all worker threads to ensure they have completed their execution before the destructor returns.
         */
        ~PoolThread() {
            {
                std::lock_guard<std::mutex> lock(_taskAccess);
                _stop = true;
            }
            _taskCounter.notify_all(); 

            for (std::thread &t : _threads) {
                if (t.joinable())
                    t.join();
            }
        }

        /** @brief Worker loop for executing tasks from the queue
         * This method is run by each worker thread in the pool. It continuously checks for tasks in the task queue and executes them. The worker thread waits on the counting semaphore until a task is available, and then it locks the task queue to safely retrieve and remove a task from the queue. If the stop flag is set and there are no more tasks to execute, the worker thread will exit the loop and terminate.
         */
        void workLoop() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(_taskAccess);
                    while (_tasks.empty() && !_stop)
                        _taskCounter.wait(lock);
                    if (_stop && _tasks.empty())
                        break;
                    if (!_tasks.empty()) {
                        task = std::move(_tasks.front());
                        _tasks.pop();
                    }
                }
                if (task) {
                    _activeWorkers += 1;
                    task();
                    _activeWorkers -= 1;
                }
            }
        }

        /** @brief Add a new task to the thread pool
         * @param new_task A function representing the task to be added to the pool
         * This method allows clients to add new tasks to the thread pool. It locks the task queue to safely add the new task, and then it releases the counting semaphore to signal that a new task is available for execution by the worker threads.
         */
        void addInQueue(std::function<void()> new_task) {
            {
                std::lock_guard<std::mutex> lock(_taskAccess);
                _tasks.push(new_task);
            }
            _taskCounter.notify_one();
        }

        bool isActive() {
            bool isEmpty = false;
            {
                std::lock_guard<std::mutex> lock(_taskAccess);
                isEmpty = _tasks.empty();
            }
            return !(isEmpty && _activeWorkers <= 0);
        }

        /** @brief Retourne le nombre de workers actuellement en train d'exécuter une tâche
         * @return Nombre de workers actifs
         */
        int getActiveWorkers() const {
            return _activeWorkers.load();
        }

        /** @brief Retourne le nombre de tâches en attente dans la queue
         * @return Nombre de tâches en attente
         */
        int getQueueSize() {
            std::lock_guard<std::mutex> lock(_taskAccess);
            return static_cast<int>(_tasks.size());
        }
};

#endif