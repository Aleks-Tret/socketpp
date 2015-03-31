#include <socketpp/server.hpp>

#include <algorithm>
#include <list>

namespace socketpp
{
  Server::Server(int const port, int const type, request_handler_t handler) throw (SocketException)
      : Socket(mem::make_shared<Address>("localhost", port, type)),
        server_thread_(),
        request_handler_(handler)
  { }

  void Server::start() {
    server_thread_ = func::thread(&Server::handle_connections, this);
  }

  auto client_connection = [](SOCKET client_socket, request_handler_t handler) {
    std::string req;
    Socket socket(client_socket);
    try {
      while ((req = socket.Read()).length() > 0) {
        socket.Write(handler(req));
      }
    }
    catch (...) { }
  };

  struct thread_deleter {
    void operator()(async::thread* t) {
      if (t->joinable())
        t->join();
      delete t;
    }
  };

  void Server::handle_connections() {
    SOCKET client_sock;
    using thread_ptr = mem::unique_ptr<std::thread, thread_deleter>;
    std::list<thread_ptr> connections;
    try {
      while ((client_sock = Accept()) != INVALID_SOCKET) {
        connections.push_back(thread_ptr(new async::thread(client_connection, client_sock, request_handler_)));
      }
    }
    catch (...) {}
  }
}
