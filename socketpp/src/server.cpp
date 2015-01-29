#include <socketpp/server.hpp>

#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstring>


namespace socketpp
{
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

  struct Connection {
    std::shared_ptr<Socket> socket;
    std::shared_ptr<std::thread> handler;
  };

  void close_connection(Connection conn) {
    conn.socket->close();
    if (conn.handler->joinable())
      conn.handler->join();
  }

  void remove_closed_connections(std::list<Connection>& clients)
  {
    clients.remove_if([](Connection connection) {
      if (connection.socket->closed() && connection.handler->joinable())
        connection.handler->join();
      return connection.socket->closed();
    });
  }

  void close_and_remove_oldest_connection(std::list<Connection>& clients)
  {
    close_connection(clients.front());
    clients.pop_front();
  }

  void close_all_connections(std::list<Connection>& clients) {
    std::for_each(clients.begin(), clients.end(), [](Connection conn) {
      close_connection(conn);
    });
    clients.clear();
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

  void Server::handle_connections() {
    std::list<Connection> clients;
    shutdown_.store(false);
    try {
      while (!shutdown_.load()) {
        remove_closed_connections(clients);
        clients.push_back(wait_incoming_connection());
        if (clients.size() > pool_size_)
          close_and_remove_oldest_connection(clients);
      }
    }
    catch (...) {
    }
    close_all_connections(clients);
  }

  void handle_connection(std::shared_ptr<Socket> socket, request_handler_t& handler) {
    std::string req;
    try {
      while ((!socket->closed()) && ((req = socket->read()).length() > 0)) {
        socket->write(handler(req));
      }
    }
    catch(...) {
    }
    socket->close();
    return;
  }

  Connection Server::wait_incoming_connection() {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    Connection client;
    client.socket = std::make_shared<Socket>(accept(socket_.load(), reinterpret_cast<struct sockaddr*>(&client_addr), &addr_size));
    client.handler = std::make_shared<std::thread>(&handle_connection, client.socket, std::ref(request_handler_));
    return client;
  }
}
