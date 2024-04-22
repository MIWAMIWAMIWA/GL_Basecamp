#ifndef GENERAL_H
#define GENERAL_H

#ifdef _WIN32
#else
#define SD_BOTH 0
#include <sys/socket.h>
#endif


#include <cstdint>
#include <cstring>
#include <cinttypes>
#include <malloc.h>

#include <queue>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

/// Simple TCP
namespace stcp {

//Platform-specific typedefs and enums 
#ifdef _WIN32 // Windows 

typedef int SockLen_t;

typedef SOCKADDR_IN SocketAddr_in;

typedef SOCKET Socket;

typedef u_long ka_prop_t;

#else // linux

typedef socklen_t SockLen_t;

typedef struct sockaddr_in SocketAddr_in;

typedef int Socket;

typedef int ka_prop_t;

#endif


constexpr uint32_t LOCALHOST_IP = 0x0100007f;


enum class SocketStatus : uint8_t {

  connected = 0,

  err_socket_init = 1,

  err_socket_bind = 2,

  err_socket_connect = 3,
  
  disconnected = 4
};
//

//simple threadpool implementation for our tcpserver

class ThreadPool {
  std::vector<std::thread> thread_pool;//container for worker threads
  std::queue<std::function<void()>> job_queue;// queue for storing jobs 
  std::mutex queue_mtx;// protecting the job queue
  std::condition_variable condition;//for synchronization
  std::atomic<bool> pool_terminated = false;


  //init threads 
  void setupThreadPool(uint thread_count) {
    thread_pool.clear();
    for(uint i = 0; i < thread_count; ++i)
      thread_pool.emplace_back(&ThreadPool::workerLoop, this);
  }
  
  //worker function executed by each thread 
  void workerLoop() {
    std::function<void()> job;
    while (!pool_terminated) {
      {
        std::unique_lock lock(queue_mtx);
        //wait for job or termination signal
        condition.wait(lock, [this](){return !job_queue.empty() || pool_terminated;});
        if(pool_terminated) return;
        // get and exec next job
        job = job_queue.front();
        job_queue.pop();
      }
      job();
    }
  }
public:
  ThreadPool(uint thread_count = std::thread::hardware_concurrency()) {setupThreadPool(thread_count);}

  ~ThreadPool() {
    pool_terminated = true;
    join();
  }
  //function to add  a job to queue
  template<typename F>
  void addJob(F job) {
    if(pool_terminated) return;
    {
      std::unique_lock lock(queue_mtx);
      job_queue.push(std::function<void()>(job));
    }
    condition.notify_one();
  }

  //function to add  a job with arguments to queue
  template<typename F, typename... Arg>
  void addJob(const F& job, const Arg&... args) {addJob([job, args...]{job(args...);});}
  //join all threads
  void join() {for(auto& thread : thread_pool) thread.join();}
  //
  uint getThreadCount() const {return thread_pool.size();}

  void dropUnstartedJobs() {
    pool_terminated = true;
    join();
    pool_terminated = false;
    // Clear jobs in queue
    std::queue<std::function<void()>> empty;
    std::swap(job_queue, empty);
    // reset thread pool
    setupThreadPool(thread_pool.size());
  }

  void stop() {
    pool_terminated = true;
    join();
  }

  void start(uint thread_count = std::thread::hardware_concurrency()) {
    if(!pool_terminated) return;
    pool_terminated = false;
    setupThreadPool(thread_count);
  }

};

typedef std::vector<uint8_t> DataBuffer;

enum class SocketType : uint8_t {
  client_socket = 0,
  server_socket = 1
};

class TcpClientBase {
public:
  typedef SocketStatus status;

  virtual ~TcpClientBase() {};


  virtual status disconnect() = 0;

  virtual status getStatus() const = 0;

  virtual bool sendData(const void* buffer, const size_t size) const = 0;

  virtual DataBuffer loadData() = 0;

  virtual uint32_t getHost() const = 0;

  virtual uint16_t getPort() const = 0;

  virtual SocketType getType() const = 0;
};

#ifdef _WIN32 // Windows
namespace {
class _WinSocketIniter {
  static WSAData w_data;
public:
  _WinSocketIniter() {
    WSAStartup(MAKEWORD(2, 2), &w_data) //init winsock
  }

  ~_WinSocketIniter() {
    WSACleanup() //cleanup winsock
  }
};

static inline _WinSocketIniter _winsock_initer; //winsock initializer
}
#endif

}

#endif // GENERAL_H