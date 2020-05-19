/*!
* \file
* \brief queue.h foo
*/

#ifndef QUEUE_H
#define QUEUE_H

#include "logger.h"

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <queue>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>
 
template <typename T>
class Queue
{
public:
    /*!
     * \brief Queue::Queue
     * Constructor
     */
    Queue()
    {
        m_eventFd = eventfd(0, 0);
    }

    /*!
     * \brief Queue::~Queue
     * Destructor
     */
    ~Queue()
    {
        close(m_eventFd);
    }

    /*!
     * \brief Queue::getEventFd
     * Get event file descriptor for queue events
     * \return Event file descriptor
     */
    int getEventFd() const
    {
        return m_eventFd;
    }

    /*!
     * \brief Queue::pop
     * Get first item from queue
     * \return First item in queue
     */
    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty()) {
            m_cond.wait(lock);
        }
        auto item = m_queue.front();
        m_queue.pop();
        return item;
    }

    /*!
     * \brief Queue::pop
     * Get first item from queue
     * \param Reference to assign first item from queue
     */
    void pop(T& item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty()) {
            m_cond.wait(lock);
        }
        item = m_queue.front();
        m_queue.pop();
    }

    /*!
     * \brief Queue::push
     * Add new item to end of queue
     * \param Reference to new item
     */
    void push(const T& item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(item);
        uint64_t val = 1;
        write(m_eventFd, &val, sizeof(uint64_t));
        lock.unlock();
        m_cond.notify_one();
    }

    /*!
     * \brief Queue::push
     * Add new item to end of queue
     * \param Reference to reference of new item
     */
    void push(T&& item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(std::move(item));
        uint64_t val = 1;
        write(m_eventFd, &val, sizeof(uint64_t));
        lock.unlock();
        m_cond.notify_one();
    }

    /*!
     * \brief Queue::empty
     * Check if queue is empty
     * \param True if queue is empty, otherwise false.
     */
    bool empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

private:
    std::condition_variable m_cond;
    std::mutex m_mutex;
    std::queue<T> m_queue;
    int m_eventFd;
};

#endif // QUEUE_H
