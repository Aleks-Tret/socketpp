#include <socketpp/server.hpp>

#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstring>

#if defined(_WIN32) && !defined(__INTIME__)
#define CHECK_STATUS(st) if ((st) != 0) goto error;
#else
#define CHECK_STATUS(s) if ((s) < 0 ) goto error;
#endif
#define CHECK_SOCKET(so) if ((so) == INVALID_SOCKET) goto error;



namespace socketpp
{
  template<typename T>
  std::string to_string(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }

  Server::Server(int const port, int const type, request_handler_t handler, size_t const pool_size) throw (SocketException) :
    Socket(INVALID_SOCKET),
    host_info_(nullptr),
    pool_size_(pool_size),
    request_handler_(handler),
    shutdown_(true)
  {
    struct addrinfo hint;
    memset(reinterpret_cast<char*>(&hint), 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = type;
    hint.ai_flags = AI_PASSIVE;
    CHECK_STATUS(getaddrinfo(NULL, to_string(port).c_str(), &hint, &host_info_));
    return;
  error:
    close();
    throw SocketException();

  }

  Server::~Server() {
    if (host_info_ != nullptr)
      freeaddrinfo(host_info_);
    host_info_ = nullptr;
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

  void Server::start() throw (SocketException) {
    int yes = 1;
    socket_ = socket(host_info_->ai_family, host_info_->ai_socktype, host_info_->ai_protocol);
    CHECK_SOCKET(socket_);
    CHECK_STATUS(setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)));
    CHECK_STATUS(bind(socket_, host_info_->ai_addr, host_info_->ai_addrlen));
    CHECK_STATUS(listen(socket_, pool_size_));
    server_thread_ = std::thread(&Server::handle_connections, this);
    return;
  error:
#if defined(_WIN32) && !defined(__INTIME__)
    int status = WSAGetLastError();
#else
    int status = errno;
#endif
    std::cout << "Error while creating socket: " << status << std::endl;
    close();
    throw SocketException();
  }

  void Server::stop() {
    shutdown_.store(true);
    close();
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
    catch(...) {
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
    client.socket = std::make_shared<Socket>(accept(socket_, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_size));
    client.handler = std::make_shared<std::thread>(&handle_connection, client.socket, std::ref(request_handler_));
    return client;
  }
}
