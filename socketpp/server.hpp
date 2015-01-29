#pragma once

#include "socket.hpp"

#include <functional>
#include <thread>
#include <string>
#include <list>
#include <memory>
#include <atomic>

namespace socketpp {
  
  typedef std::function<std::string(std::string)> request_handler_t;

  struct Connection;

  class Server : public Socket {
    public:
      Server(int const port, int const type, request_handler_t handler, int const pool_size = 5) throw (SocketException);
      Server(Server const &) = delete;
      Server operator=(Server const&) = delete;
      ~Server();

      void start();

    private:
      int pool_size_;

      std::thread server_thread_;
      request_handler_t request_handler_;

      std::atomic<bool> shutdown_;

      void handle_connections();
      // TODO: This method should be declared only in .cpp file
      Connection wait_incoming_connection();

      void stop();
  };
}

