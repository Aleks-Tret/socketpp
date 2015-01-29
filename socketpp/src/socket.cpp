#include <socketpp/socket.hpp>

#include <cstring>
#include <string>
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
    struct addrinfo* host_info;
    int status = 0;
    memset(reinterpret_cast<char*>(&hint), 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = type;
    hint.ai_flags = AI_PASSIVE;
    CHECK_STATUS(getaddrinfo(NULL, std::to_string(port).c_str(), &hint, &host_info));
    int yes = 1;
    socket_.store(socket(host_info->ai_family, host_info->ai_socktype, host_info->ai_protocol));
    CHECK_SOCKET(socket_.load());
    CHECK_STATUS(setsockopt(socket_.load(), SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)));
    CHECK_STATUS(bind(socket_.load(), host_info->ai_addr, host_info->ai_addrlen));
    goto cleanup;
  error:
    status = report_socket_error();
    close();
  cleanup:
    if (host_info != nullptr)
      freeaddrinfo(host_info);
    host_info = nullptr;
    if (status != 0) throw SocketException();
  }

  Socket::~Socket() {
    close();
  }

  void Socket::close() {
    shutdown(socket_.load(), BOTH_DIRECTION);
    closesocket(socket_.load());
    socket_.store(INVALID_SOCKET);
  }

  void Socket::write(std::string&& msg) {
    if (send(socket_.load(), msg.c_str(), msg.length(), 0) != msg.length())
      close();
  }

  bool Socket::closed() {
    return socket_.load() == INVALID_SOCKET;
  }

  std::string Socket::read() {
    const auto MAX_SIZE = 4096;
    char client_message[MAX_SIZE];
    memset(client_message, 0, MAX_SIZE);
    auto complete_read_size = recv(socket_.load(), &client_message[0], MAX_SIZE, 0);
    if (complete_read_size <= 0) {
      close();
      return "";
    }
    return std::string(client_message, static_cast<size_t>(complete_read_size));
  }
}

