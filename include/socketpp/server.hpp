#pragma once

#include <socketpp.hpp>

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
      Server(int const port, int const type, request_handler_t handler, size_t const pool_size = 5) throw (SocketException);
      Server(Server const &) = delete;
      Server operator=(Server const&) = delete;
      ~Server() override;

      void start() throw (SocketException);
      void stop();

    private:
      struct addrinfo* host_info_;
      size_t pool_size_;

      std::thread server_thread_;
      request_handler_t request_handler_;

      std::atomic<bool> shutdown_;

      void handle_connections();
      
      Connection wait_incoming_connection();
  };
}

