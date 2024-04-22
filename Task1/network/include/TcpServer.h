#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <functional>
#include <list>

#include <thread>
#include <mutex>
#include <shared_mutex>

#ifdef _WIN32 // Windows 
#include <WinSock2.h>
#include <mstcpip.h>
#else // linux

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include "general.h"

// Simple TCP
namespace stcp {

//Struct to configure TCP Keep _alive settings
struct KeepAliveConfig{
  //time to idle to send keepalive packet
  ka_prop_t ka_idle = 120;
  //interval between keepalive packets
  ka_prop_t ka_intvl = 3;
  //number of keep alive packets
  ka_prop_t ka_cnt = 5;
};

//Forward declaration of TCPserver 
struct TcpServer {

  struct Client;

  class ClientList;

  //type definitions for handler fuctions
  typedef std::function<void(DataBuffer, Client&)> handler_function_t;

  typedef std::function<void(Client&)> con_handler_function_t;

  //default handler functions 
  static constexpr auto default_data_handler = [](DataBuffer, Client&){};

  static constexpr auto default_connsection_handler = [](Client&){};

  //enum for tcp server status 

  enum class status : uint8_t {

    up = 0,
    //error while socket init
    err_socket_init = 1,
    //error when socket binding
    err_socket_bind = 2,
    //error when configuring keep alive settings
    err_scoket_keep_alive = 3,
    //error setting socket for listening
    err_socket_listening = 4,

    close = 5
  };

private:
  //private members of server
  Socket serv_socket;

  uint16_t port;

  status _status = status::close;

  handler_function_t handler = default_data_handler;

  con_handler_function_t connect_hndl = default_connsection_handler;

  con_handler_function_t disconnect_hndl = default_connsection_handler;
  //thread pool that allows making this server multithreaded
  ThreadPool thread_pool;

  KeepAliveConfig ka_conf;

  std::list<std::unique_ptr<Client>> client_list;
  
  std::mutex client_mutex;

  typedef std::list<std::unique_ptr<Client>>::iterator ClientIterator;

  //private methods for managing server-client connections


  bool enableKeepAlive(Socket socket);

  void handlingAcceptLoop();

  void waitingDataLoop();

public:

  TcpServer(const uint16_t port,
            KeepAliveConfig ka_conf = {},
            handler_function_t handler = default_data_handler,
            con_handler_function_t connect_hndl = default_connsection_handler,
            con_handler_function_t disconnect_hndl = default_connsection_handler,
            uint thread_count = std::thread::hardware_concurrency()
            );

  ~TcpServer();

  //! Set client handler
  void setHandler(handler_function_t handler);

  ThreadPool& getThreadPool() {return thread_pool;}

  // Server properties getters
  uint16_t getPort() const;

  uint16_t setPort(const uint16_t port);

  status getStatus() const {return _status;}

  // Server status manipulation
  status start();

  void stop();

  void joinLoop();

  // Server client management
  bool connectTo(uint32_t host, uint16_t port, con_handler_function_t connect_hndl);

  void sendData(const void* buffer, const size_t size);

  bool sendDataBy(uint32_t host, uint16_t port, const void* buffer, const size_t size);

  bool disconnectBy(uint32_t host, uint16_t port);

  void disconnectAll();

};

//struct representing a client connected to server

struct TcpServer::Client : public TcpClientBase {

  friend struct TcpServer;

  std::mutex access_mtx;

  SocketAddr_in address;

  Socket socket;

  status _status = status::connected;


public:

  Client(Socket socket, SocketAddr_in address);

  virtual ~Client() override;

  virtual uint32_t getHost() const override;

  virtual uint16_t getPort() const override;

  virtual status getStatus() const override {return _status;}

  virtual status disconnect() override;

  virtual DataBuffer loadData() override;

  virtual bool sendData(const void* buffer, const size_t size) const override;

  virtual SocketType getType() const override {return SocketType::server_socket;}
};

}

#endif