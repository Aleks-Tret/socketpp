#include <socketpp/server.hpp>

#include <algorithm>
#include <list>

namespace socketpp
{
  Server::Server(int const port, int const type, request_handler_t handler) throw (SocketException)
      : socket_(port, type),
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
    socket.close();
  };

  void Server::handle_connections() {
    SOCKET client_sock;
    std::list<std::thread> connections;
    try {
      while ((client_sock = socket_.accept()) != INVALID_SOCKET) {
        connections.push_back(std::thread(client_connection, client_sock, request_handler_));
      }
    }
    catch (...) { }
    std::for_each(connections.begin(), connections.end(), [](std::thread & connection) {
      connection.join();
    });
    connections.clear();
  }
}
