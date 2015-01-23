#include <socketpp.hpp>

#include <sstream>

#if defined(_WIN32) && !defined(__INTIME__)
  #define CHECK_STATUS(st) if ((st) == SOCKET_ERROR) goto error;
#else
  #define CHECK_STATUS(s) if ((s) < 0 ) goto error;
#endif

#define CHECK_SOCKET(so) if ((so) == INVALID_SOCKET) goto error;

template<typename T>
std::string to_string(const T& value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

namespace socketpp {

  Socket::Socket(int const &socket) : socket_(socket)
  { 
    if(socket_ == INVALID_SOCKET)
      throw SocketException();
  }

  Socket::~Socket() {
    close();
  }

  void Socket::close() {
    if (socket_ != INVALID_SOCKET) {
      shutdown(socket_, BOTH_DIRECTION);
      closesocket(socket_);
    }
    socket_ = INVALID_SOCKET;
  }

  bool Socket::write(std::string msg) {
    return send(socket_, msg.c_str(), msg.length(), 0) == msg.length();
  }

  std::string Socket::read() {
    const int MAX_SIZE = 1000;
    char client_message[MAX_SIZE];
    memset(client_message, 0, MAX_SIZE);
    int read_size = recv(socket_, client_message , MAX_SIZE, 0);
    return std::string(client_message, read_size);
  }


  ServerSocket::ServerSocket(int const port, int const type) throw (SocketException) :
      Socket(INVALID_SOCKET),
      host_info_(nullptr)
  {
    struct addrinfo hint;
    memset((char*)&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = type;
    hint.ai_flags = AI_PASSIVE;
    CHECK_STATUS(getaddrinfo(NULL, to_string(port).c_str(), &hint, &host_info_));
    return;
error:
    close();
    throw SocketException();

  }

  ServerSocket::~ServerSocket() {
    if (host_info_ != nullptr)
      freeaddrinfo(host_info_);
    host_info_ = nullptr;
  }

  void ServerSocket::start() throw (SocketException) {
    char yes = 1;
    socket_ = socket(host_info_->ai_family, host_info_->ai_socktype, host_info_->ai_protocol);
    CHECK_SOCKET(socket_);
    CHECK_STATUS(setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)));
    CHECK_STATUS(bind(socket_, host_info_->ai_addr, host_info_->ai_addrlen));
    CHECK_STATUS(listen(socket_, 1));
    return;
error:
    close();
    throw SocketException();
  }


  Socket ServerSocket::wait_connection() {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    return Socket(accept(socket_, (struct sockaddr*)&client_addr, &addr_size));
  }
}

