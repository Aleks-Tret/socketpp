#pragma once

#if defined(_WIN32)
# if !defined(__INTIME__)
#  include <WinSock2.h>
#  include <ws2tcpip.h>
#  include <io.h>
static const int BOTH_DIRECTION=SD_BOTH;
#  define CHECK_STATUS(st) if ((st) != 0) goto error;
# endif
#  pragma warning(disable:4290)
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <unistd.h>
  static const int BOTH_DIRECTION=SHUT_RDWR;
  static int const INVALID_SOCKET = -1;
  typedef int SOCKET;
# define CHECK_STATUS(s) if ((s) < 0 ) goto error;
# define closesocket(s) close((s))
#endif
#define CHECK_SOCKET(so) if ((so) == INVALID_SOCKET) goto error;

#include <socketpp/unique_handler.hpp>

#include <string>


namespace socketpp {

  class Address {
    public:
      Address(std::string const, unsigned short const port = 8888, int const type = SOCK_STREAM);
      operator addrinfo*() { return addr_info.get(); }

    private:
      std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> addr_info;
  };

  class Socket {
    public:
      Socket(std::shared_ptr<Address>);
      Socket(SOCKET const&);
      Socket(Socket const&) = delete;
      Socket& operator=(Socket const &) = delete;
      virtual ~Socket() = default;

      void Write(std::string &&);
      std::string Read();
      SOCKET Accept();

    protected:
      static inline void del_SOCKET(SOCKET s) {
        shutdown(s, BOTH_DIRECTION);
        closesocket(s);
      }
      class unique_SOCKET : public std::unique_ptr<SOCKET, OneLinerDeleter<SOCKET, INVALID_SOCKET, void, del_SOCKET>> {
      public:
        operator SOCKET() const {
          return get();
        }
        unique_SOCKET(const SOCKET& s) : std::unique_ptr<SOCKET, OneLinerDeleter<SOCKET, INVALID_SOCKET, void, del_SOCKET>>(s) {}
      };

      unique_SOCKET socket_;
  };
}

