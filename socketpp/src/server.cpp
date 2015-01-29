#include <socketpp/server.hpp>

#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstring>


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
    Socket(port, type),
    pool_size_(pool_size),
    request_handler_(handler),
    shutdown_(true),
    server_thread_(nullptr, &del_thread)
  {
  }

  Server::~Server() {
    shutdown_.store(true);
  }

  void Server::start() {
    CHECK_STATUS(listen(socket_.load(), 1));
    server_thread_ = thread_uptr_t(new std::thread(&Server::handle_connections, this), &del_thread);
    return;
  error:
    report_socket_error();
    close();
    throw SocketException();
  }

  SOCKET Server::accept() {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    return ::accept(socket_.load(), reinterpret_cast<struct sockaddr*>(&client_addr), &addr_size);
  }

  void Server::handle_connections() {
    using  connection_t = std::unique_ptr<Connection>;
    std::list<connection_t> clients;
    shutdown_.store(false);
    try {
      while (!shutdown_.load()) {
        // Remove closed function
        clients.remove_if([](connection_t& connection) {
          return connection->finished();
        });
        clients.push_back(std::make_unique<Connection>(accept(), request_handler_));
        if (clients.size() > pool_size_)
          // Remove oldest connection
          clients.pop_front();
      }
    }
    catch (...) {
    }
    clients.clear();
  }
}
