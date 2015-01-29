#include <socketpp/socket.hpp>

#include <cstring>
#include <iostream>

namespace socketpp {

  int report_socket_error() {
#if defined(_WIN32) && !defined(__INTIME__)
    int status = WSAGetLastError();
#else
    int status = errno;
#endif
    std::cout << "Error while creating socket: " << status << std::endl;
    return status;
  }

  Socket::Socket(SOCKET const& socket) : socket_(socket)
  { }

  Socket::Socket(int const port, int const type) {
    struct addrinfo hint;
    struct addrinfo* hi_p;
    memset(reinterpret_cast<char*>(&hint), 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = type;
    hint.ai_flags = AI_PASSIVE;
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    getaddrinfo(nullptr, std::to_string(port).c_str(), &hint, &hi_p);
    std::unique_ptr <struct addrinfo, decltype(&freeaddrinfo)>  host_info(hi_p, &freeaddrinfo);
    SOCKET s = socket(host_info.get()->ai_family, host_info.get()->ai_socktype, host_info.get()->ai_protocol);
    CHECK_SOCKET(s);
    int yes = 1;
    CHECK_STATUS(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&yes), sizeof(yes)));
    CHECK_STATUS(bind(s, host_info.get()->ai_addr, host_info.get()->ai_addrlen));
    CHECK_STATUS(listen(s, 1));
    socket_ = unique_SOCKET(s);
    return;
  error:
    report_socket_error();
    throw SocketException();
  }

  void Socket::write(std::string&& msg) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (send(socket_.get(), msg.c_str(), msg.length(), 0) != msg.length())
      throw SocketException();
  }

  std::string Socket::read() {
    const auto MAX_SIZE = 4096;
    char client_message[MAX_SIZE];
    memset(client_message, 0, MAX_SIZE);
    std::lock_guard<std::mutex> lock(socket_mutex_);
    auto complete_read_size = recv(socket_.get(), &client_message[0], MAX_SIZE, 0);
    if (complete_read_size <= 0)
      throw SocketException();
    return std::string(client_message, static_cast<size_t>(complete_read_size));
  }

  SOCKET Socket::accept() {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    SOCKET client;
    if((client = ::accept(socket_.get(), reinterpret_cast<struct sockaddr*>(&client_addr), &addr_size)) == INVALID_SOCKET)
      throw SocketException();
    return client;
  }
}

