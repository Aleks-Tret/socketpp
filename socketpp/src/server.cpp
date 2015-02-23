#include <socketpp/server.hpp>

#include <algorithm>
#include <list>

namespace socketpp
{
  Server::Server(int const port, int const type, request_handler_t handler) throw (SocketException)
      : Socket(port, type),
        server_thread_(),
        request_handler_(handler)
  { }

  void Server::start() {
    server_thread_ = std::thread(&Server::handle_connections, this);
  }

  auto client_connection = [](SOCKET client_socket, request_handler_t handler) {
    std::string req;
    Socket socket(client_socket);
    try {
      while ((req = socket.read()).length() > 0) {
        socket.write(handler(req));
      }
    }
    catch (...) { }
  };

  struct thread_deleter {
    void operator()(std::thread* t) {
      if (t->joinable())
        t->join();
      delete t;
    }
  };

  void Server::handle_connections() {
    SOCKET client_sock;
    using thread_ptr = std::unique_ptr<std::thread, thread_deleter>;
    std::list<thread_ptr> connections;
    try {
      while ((client_sock = accept()) != INVALID_SOCKET) {
        connections.push_back(thread_ptr(new std::thread(client_connection, client_sock, request_handler_)));
      }
    }
    catch (...) {}
  }
}
