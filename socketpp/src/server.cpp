#include <socketpp/server.hpp>

#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstring>


namespace socketpp
{
  void handle_connection(Socket& socket, request_handler_t& handler) {
    std::string req;
    try {
      while ((req = socket.read()).length() > 0) {
        socket.write(handler(req));
      }
    }
    catch (...) {
    }
    socket.close();
    return;
  }

  class Connection {
  public:
    Socket socket;
    std::thread connection_thread;

    Connection(SOCKET, request_handler_t);
    ~Connection();
    Connection(Connection const&) = delete;
    Connection& operator=(Connection const&) = delete;
  };

  Connection::Connection(SOCKET s, request_handler_t h)
    : socket(s),
      connection_thread(std::thread(&handle_connection, std::ref(socket), h))
  {
  }

  Connection::~Connection()
  {
    socket.close();
    connection_thread.join();
  }

  typedef std::unique_ptr<Connection> connection_t;
  typedef std::list<connection_t> connections_t;

  void remove_closed_connections(connections_t & clients)
  {
    clients.remove_if([](connection_t& connection) {
      return connection->socket.closed();
    });
  }

  void close_and_remove_oldest_connection(connections_t& clients)
  {
    clients.pop_front();
  }

  void close_all_connections(connections_t& clients) {
    clients.clear();
  }

  Server::Server(int const port, int const type, request_handler_t handler, int const pool_size) throw (SocketException) :
    Socket(port, type),
    pool_size_(pool_size),
    request_handler_(handler),
    shutdown_(true)
  {
  }

  Server::~Server() {
    stop();
  }

  void Server::start() {
    CHECK_STATUS(listen(socket_.load(), pool_size_));
    server_thread_ = std::thread(&Server::handle_connections, this);
    return;
  error:
    report_socket_error();
    close();
    throw SocketException();
  }

  void Server::stop() {
    shutdown_.store(true);
    close(); // To cancel *accept* command
    if (server_thread_.joinable())
      server_thread_.join();
  }

  SOCKET Server::accept() {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    return ::accept(socket_.load(), reinterpret_cast<struct sockaddr*>(&client_addr), &addr_size);
  }

  void Server::handle_connections() {
    connections_t clients;
    shutdown_.store(false);
    try {
      while (!shutdown_.load()) {
        remove_closed_connections(clients);
        clients.push_back(std::make_unique<Connection>(accept(), request_handler_));
        if (clients.size() > pool_size_)
          close_and_remove_oldest_connection(clients);
      }
    }
    catch (...) {
    }
    close_all_connections(clients);
  }
}
