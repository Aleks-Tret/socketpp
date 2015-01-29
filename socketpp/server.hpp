#pragma once

#include "socket.hpp"

#include <thread>


namespace socketpp {
  typedef std::function<std::string(std::string)> request_handler_t;

  inline void del_thread(std::thread* t) {
    t->join();
  };

  typedef std::unique_ptr < std::thread, decltype(&del_thread) > thread_uptr_t;

  class Server {
    public:
      Server(int const port, int const type, request_handler_t handler, size_t const pool_size = 5) throw (SocketException);
      Server(Server const &) = delete;
      Server operator=(Server const&) = delete;
      ~Server() { socket_.reset(); }

      void start();

    private:
      size_t pool_size_;
      std::unique_ptr<Socket> socket_;
      thread_uptr_t server_thread_;
      request_handler_t request_handler_;

      void handle_connections();
  };
}

