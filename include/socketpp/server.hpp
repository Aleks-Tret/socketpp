#pragma once

#include <socketpp/exception.hpp>

#include <functional>
#include <thread>

namespace socketpp {
  typedef std::function<std::string(std::string)> request_handler_t;

  class Server {
    public:
      Server(request_handler_t handler, Socket const & socket, int const pool_size = 5) throw (SocketException);
      Server(Server const &) = delete;
      Server operator=(Server const&) = delete;
      ~Server() {}

      void Start();
      void Stop();

    private:
      Socket socket_;
      size_t pool_size_;

      std::thread server_thread_;
  };
}

