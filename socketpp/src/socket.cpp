#include <socketpp/socket.hpp>

#include <cstring>

namespace socketpp {

  Socket::Socket(SOCKET const& socket) : socket_(socket)
  { }

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
    ssize_t complete_read_size = recv(socket_.load(), &client_message[0], MAX_SIZE, 0);
    if (complete_read_size <= 0) {
      close();
      return "";
    }
    return std::string(client_message, static_cast<size_t>(complete_read_size));
  }
}

