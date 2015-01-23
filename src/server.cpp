#include <socketpp/server.hpp>

#include <sstream>

#if defined(_WIN32) && !defined(__INTIME__)
#define CHECK_STATUS(st) if ((st) == SOCKET_ERROR) goto error;
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

  Server::Server(int const port, int const type, request_handler_t handler, int const pool_size) throw (SocketException) :
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

  void Server::remove_closed_connections()
  {
    clients_.remove_if([](Connection connection) {
      if (connection.socket->closed())
        connection.handler->join();
      return connection.socket->closed();
    });
  }

  void Server::close_and_remove_oldest_connection()
  {
    clients_.front().socket->close();
    clients_.front().handler->join();
    clients_.pop_front();
  }

  void Server::start() throw (SocketException) {
    char yes = 1;
    socket_ = socket(host_info_->ai_family, host_info_->ai_socktype, host_info_->ai_protocol);
    CHECK_SOCKET(socket_);
    CHECK_STATUS(setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)));
    CHECK_STATUS(bind(socket_, host_info_->ai_addr, host_info_->ai_addrlen));
    CHECK_STATUS(listen(socket_, 1));
    shutdown_ = false;
    server_thread_ = std::thread(&Server::handle_connections, this);
    return;
  error:
    close();
    throw SocketException();
  }

  void Server::stop()
  {
    shutdown_ = true;
  }

  void Server::handle_connections()
  {
    while (!shutdown_) {
      remove_closed_connections();
      if (clients_.size() >= pool_size_)
        close_and_remove_oldest_connection();
      clients_.push_back(wait_incoming_connection());
    }
  }

  void handle_connection(std::shared_ptr<Socket> socket, request_handler_t& handler)
  {
    std::string req;
    while ((req = socket->read()).length() > 0)
      socket->write(handler(req));
    socket->close();
  }

  Server::Connection Server::wait_incoming_connection() {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    Connection client;
    client.socket = std::make_shared<Socket>(accept(socket_, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_size));
    client.handler = std::make_shared<std::thread>(&handle_connection, client.socket, std::ref(request_handler_));
    return client;
  }
}