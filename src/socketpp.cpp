#include <socketpp.hpp>

namespace socketpp {

  Socket::Socket(SOCKET const& socket) : socket_(socket)
  { 
    if(socket_ == INVALID_SOCKET)
      throw SocketException();
  }

  Socket::~Socket() {
    close();
  }

  void Socket::close() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (socket_ != INVALID_SOCKET) {
      shutdown(socket_, BOTH_DIRECTION);
      closesocket(socket_);
    }
    socket_ = INVALID_SOCKET;
  }

  void Socket::write(std::string msg) throw (SocketException) {
    size_t written_size = [this](std::string& m) -> size_t
    {
      std::lock_guard<std::mutex> lock(socket_mutex_);
      return static_cast<size_t>(send(socket_, m.c_str(), m.length(), 0));
    }(msg);
    if (written_size == 0)
      close();
    if (written_size != msg.length())
      throw SocketException();
  }

  std::string Socket::read() {
    const auto MAX_SIZE = 1000;
    char client_message[MAX_SIZE];
    memset(client_message, 0, MAX_SIZE);
    size_t read_size = [this](char* msg)
    {
      std::lock_guard<std::mutex> lock(socket_mutex_);
      return recv(socket_, msg, sizeof(msg), 0);
    }(client_message);
    if (read_size == 0)
      close();
    return std::string(client_message, read_size);
  }
}

