#include "network/include/TcpClient.h"
#include "platform/mousefunc.cpp"
#include <iostream>
#include <stdlib.h>
#include <thread>

using namespace stcp;

void runClient(TcpClient& client) {

  using namespace std::chrono_literals;
  if(client.connectTo(LOCALHOST_IP, 5577) == SocketStatus::connected) {
      std::clog << "Client connected\n";
        client.setHandler([&client](DataBuffer data) {
          //it receives data from server 
          std::clog << "Recived " << data.size() << " bytes: " << (char*)data.data() << '\n';
          //after this we have segmentation fault
          if(check_movement()){
            client.sendData("active\0", sizeof ("active\0"));
          }else{
            client.sendData("passive\0", sizeof ("passive\0"));
          }       
        });
        client.sendData("Hello, server\0", sizeof ("Hello, server\0"));//this sends to server 
  } else {
    std::cerr << "Client isn't connected\n";
    std::exit(EXIT_FAILURE);
  }
}

int main(int, char**) {

  ThreadPool thread_pool;

  TcpClient first_client(&thread_pool);

  runClient(first_client);

  first_client.joinHandler();

  return EXIT_SUCCESS;
}