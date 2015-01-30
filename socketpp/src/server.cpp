#include <socketpp/server.hpp>

#include <algorithm>
#include <cstring>
#include <atomic>
#include <list>

namespace socketpp
{
  class Connection {
  public:
    Connection(SOCKET&, request_handler_t&);
    ~Connection() = default;
    Connection(Connection const&) = delete;
    Connection& operator=(Connection const&) = delete;

    bool finished() {
      return socket_ ? false : true;
    }
    
  private:
    unique_Socket_t socket_;
    request_handler_t request_handler_;
    thread_uptr_t connection_thread_;

    void handle_connection();
  };

  Connection::Connection(SOCKET& s, request_handler_t& h)
    : socket_(new Socket(s), &del_Socket),
      request_handler_(h),
      connection_thread_(new std::thread(&Connection::handle_connection, this), &del_thread)
  { }

  void Connection::handle_connection() {
    std::string req;
    try {
      while (socket_ && ((req = socket_->read()).length() > 0)) {
        auto resp = request_handler_(req);
        if (socket_)
          socket_->write(request_handler_(req));
      }
    }
    catch (...) {
    }
    socket_.reset();
  }

  Server::Server(int const port, int const type, request_handler_t handler, size_t const pool_size) throw (SocketException)
      : pool_size_(pool_size),
        socket_(new Socket(port, type), &del_Socket),
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
      while (socket_ && ((client_sock = socket_->accept()) != INVALID_SOCKET)) {
        clients.push_back(std::unique_ptr<Connection>(new Connection(client_sock, request_handler_)));
        // Remove closed function
        clients.remove_if([](connection_t& connection) {
          return connection->finished();
        });
        if (clients.size() > pool_size_) {
          // Remove oldest connection
          clients.pop_front();
        }
      }
    }
    catch (...) {
    }
    socket_.reset();
    clients.clear();
  }
}
