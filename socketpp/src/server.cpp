#include <socketpp/server.hpp>

#include <algorithm>
#include <cstring>
#include <atomic>
#include <list>

namespace socketpp
{
  void handle_connection(Socket& socket, request_handler_t& handler, std::atomic<bool>& finished) {
    std::string req;
    try {
      while ((req = socket.read()).length() > 0) {
        socket.write(handler(req));
      }
    }
    catch (...) {
    }
    finished.store(true);
  }

  class Connection {
  public:
    Connection(SOCKET, request_handler_t);
    ~Connection() = default;
    Connection(Connection const&) = delete;
    Connection& operator=(Connection const&) = delete;

    bool finished() {
      return finished_.load();
    }

  private:
    Socket socket_;
    std::atomic<bool> finished_;
    thread_uptr_t connection_thread_;
  };

  Connection::Connection(SOCKET s, request_handler_t h)
    : socket_(s),
      finished_(false),
      connection_thread_(new std::thread(&handle_connection, std::ref(socket_), h, std::ref(finished_)), &del_thread)
  { }

  Server::Server(int const port, int const type, request_handler_t handler, size_t const pool_size) throw (SocketException) :
    pool_size_(pool_size),
    socket_(port, type),
    server_thread_(nullptr, &del_thread),
    request_handler_(handler)
  { }

  void Server::start() {
    server_thread_ = thread_uptr_t(new std::thread(&Server::handle_connections, this), &del_thread);
  }

  void Server::handle_connections() {
    using  connection_t = std::unique_ptr<Connection>;
    std::list<connection_t> clients;
    SOCKET client_sock;
    try {
      // *accept* returning INVALID_SOCKET means socket has been closed
      while ((client_sock = socket_.accept()) != INVALID_SOCKET) {
        clients.push_back(std::make_unique<Connection>(client_sock, request_handler_));
        // Remove closed function
        clients.remove_if([](connection_t& connection) {
          return connection->finished();
        });
        if (clients.size() > pool_size_)
          // Remove oldest connection
          clients.pop_front();
      }
    }
    catch (...) {
    }
  }
}
