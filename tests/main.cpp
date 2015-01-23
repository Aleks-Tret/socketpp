//#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
//#include "catch.hpp"


#include <socketpp/server.hpp>

const int ESCAPE = 27;

int main(int argc, char* argv[])
{
#if defined(_WIN32) && !defined(__INTIME__)
  WSADATA wsaData;
  int err;
  WORD wVersionRequested;
  /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
  wVersionRequested = MAKEWORD(2, 2);

  err = WSAStartup(wVersionRequested, &wsaData);
#endif

  socketpp::Server server(8888, SOCK_STREAM, [](std::string req) {return req; });
  server.start();
  while (std::getchar() != ESCAPE) {
  }
  server.stop();
#if defined(_WIN32) && !defined(__INTIME__)
  WSACleanup();
#endif
}


