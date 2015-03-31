#pragma once

#include <socketpp/socket.hpp>
#include <socketpp/exception.hpp>



#ifdef __INTIME__
# include <boost/function.hpp>
  namespace func = boost;

#include <boost/thread.hpp>
  namespace async = boost;
#else
# include <functional>
  namespace func = std;

# include <thread>
  namespace async = std;
#endif


namespace socketpp {
  typedef func::function<std::string(std::string)> request_handler_t;

  class Server : public Socket {
    public:
      Server(int const port, int const type, request_handler_t handler) throw (SocketException);
      Server(Server const &) = delete;
      Server operator=(Server const&) = delete;
      ~Server() {
          socket_.reset(INVALID_SOCKET);
          server_thread_.join();
      }
      void start();

    private:
      async::thread server_thread_;
      request_handler_t request_handler_;

      void handle_connections();
  };
}

