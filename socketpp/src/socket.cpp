#include <socketpp/socket.hpp>
#include <socketpp/exception.hpp>

#include <iostream>
#include <cstring>

namespace socketpp {

  Address::Address(std::string const host, unsigned short const port, int const type)
      : addr_info(nullptr, &freeaddrinfo)
  {
    struct addrinfo hint, *info;
    memset(reinterpret_cast<char*>(&hint), 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = type;
    hint.ai_flags = AI_PASSIVE;
    getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hint, &info);
    addr_info.reset(info);
  }

  // TODO: Split socket creation in Address definition and Socket creation (http://reasoning.biz/examples.htm#Networking)
  SOCKET CreateSocket(Address& address) {
    static int yes = 1;
    addrinfo* addr = address.operator addrinfo*();
    SOCKET s = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    CHECK_SOCKET(s);
    CHECK_STATUS(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&yes), sizeof(yes)));
    CHECK_STATUS(bind(s, addr->ai_addr, addr->ai_addrlen));
    CHECK_STATUS(listen(s, 1));
    return s;
  error:
    throw SocketException();
  }

  Socket::Socket(SOCKET const& socket)
    : socket_(socket)
  { }

  Socket::Socket(std::shared_ptr<Address> addr) : socket_(CreateSocket(*addr)){

  }

  void Socket::Write(std::string &&msg) {
    if (send(socket_, msg.c_str(), msg.length(), 0) != msg.length())
      throw SocketException();
  }

  std::string Socket::Read() {
    const auto MAX_SIZE = 4096;
    char client_message[MAX_SIZE];
    memset(client_message, 0, MAX_SIZE);
    auto complete_read_size = recv(socket_, &client_message[0], MAX_SIZE, 0);
    if (complete_read_size <= 0)
      throw SocketException();
    return std::string(client_message, static_cast<size_t>(complete_read_size));
  }

  SOCKET Socket::Accept() {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    SOCKET client(::accept(socket_, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_size));
    if(!client)
      throw SocketException();
    return client;
  }
}
