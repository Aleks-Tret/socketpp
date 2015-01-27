#include <socketpp/socket.hpp>

#include <fcntl.h>
#include <cstring>

namespace socketpp {

  Socket::Socket(SOCKET const& socket) : socket_(socket)
  { 

  }

  Socket::~Socket() {
    close();
  }

  void Socket::close() {

    if (!closed()) {
      std::lock_guard<std::mutex> lock(socket_mutex_);
      shutdown(socket_, BOTH_DIRECTION);
      closesocket(socket_);
      socket_ = INVALID_SOCKET;
    }

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

  void Socket::set_non_blocking(bool )
  {
    std::lock_guard<std::mutex> lock(socket_mutex_);
#ifdef _WIN32
      u_long non_blocking_socket = 1;
      ioctlsocket(socket_, FIONBIO, &non_blocking_socket);
#else
      int flags;
      if (-1 == (flags = fcntl(socket_, F_GETFL, 0)))
        flags = 0;
      fcntl(socket_, F_SETFL, flags | O_NONBLOCK);
#endif
  }

  bool Socket::closed()
  {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    return socket_ == INVALID_SOCKET;
  }

  std::string Socket::read() {
    const auto MAX_SIZE = 4096;
    char client_message[MAX_SIZE];
    memset(client_message, 0, MAX_SIZE);

    int complete_read_size = [this, MAX_SIZE](char* msg) {
      std::lock_guard<std::mutex> lock(socket_mutex_);
      return  recv(socket_, &msg[0], MAX_SIZE, 0);
    }(client_message);
    if (complete_read_size == 0) {
      close();
    }
    if (complete_read_size > 0)
      return std::string(client_message, static_cast<size_t>(complete_read_size));
    else
      return "";
  }
}

