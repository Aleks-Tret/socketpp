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

  inline void del_thread(std::thread* t) {
    t->join();
  };

  typedef std::unique_ptr < std::thread, decltype(&del_thread) > thread_uptr_t;

  class Server : public Socket {
    public:
      Server(int const port, int const type, request_handler_t handler, size_t const pool_size = 5) throw (SocketException);
      Server(Server const &) = delete;
      Server operator=(Server const&) = delete;
      ~Server();

      void start();

    private:
      size_t pool_size_;

      thread_uptr_t server_thread_;
      request_handler_t request_handler_;

      std::atomic<bool> shutdown_;

      void handle_connections();

      SOCKET accept();
  };
}

