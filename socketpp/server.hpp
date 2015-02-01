#pragma once

#include <socketpp/socket.hpp>
#include <socketpp/exception.hpp>

#include <thread>


namespace socketpp {
  typedef std::function<std::string(std::string)> request_handler_t;

  class Server {
    public:
      Server(int const port, int const type, request_handler_t handler) throw (SocketException);
      Server(Server const &) = delete;
      Server operator=(Server const&) = delete;
      ~Server() {
          socket_.close();
          server_thread_.join();
      }
      void start();

    private:
      Socket socket_;
      std::thread server_thread_;
      request_handler_t request_handler_;

      void handle_connections();
  };
}

