#pragma once

#if defined(_WIN32) && !defined(__INTIME__)
  #include <WinSock2.h>
  #include <ws2tcpip.h>
  #include <io.h>
  static const int BOTH_DIRECTION=SD_BOTH;
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  static const int BOTH_DIRECTION=SHUT_RDWR;
  static int const INVALID_SOCKET = -1;
  #ifndef __INTIME__
    #define closesocket(s) close((s))
  #endif
#endif

#include <socketpp/exception.hpp>

#include <string>

namespace socketpp {

  class Socket {
    public:
      Socket(int const & socket);
      Socket(Socket const&) = delete;
      Socket& operator=(Socket const &) = delete;
      virtual ~Socket();

      void close();
      bool write(std::string);
      std::string read();

    protected:
      int socket_;
  };

  class ServerSocket : public Socket {
      ServerSocket(int const port = 8080, int const type = SOCK_STREAM) throw (SocketException);
      ServerSocket(ServerSocket const &) = delete;
      ServerSocket& operator=(ServerSocket const&) = delete;
      ~ServerSocket();
      
      void start() throw (SocketException);
      Socket wait_connection();

    private:
      struct addrinfo* host_info_;
  };
}

