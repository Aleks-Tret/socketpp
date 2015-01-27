#include <catch.hpp>

#include <socketpp/server.hpp>

#include <string>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <future>
#include <numeric>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

std::string exec(std::string cmd) {
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != nullptr)
      result += buffer;
  }
  pclose(pipe);
  return std::string(result.begin(), std::remove(result.begin(), result.end(), '\n'));
}

std::function<std::string(unsigned int)> get_msg(std::string msg) {
  return [msg](unsigned int id) {
    return msg + "-" + std::to_string(id);
  };
}

std::list<std::pair<unsigned int, std::future<std::string>>> send_requests_and_check_result(std::string msg, size_t nb_messages, std::string address, unsigned int port) {
  std::list<std::pair<unsigned int, std::future<std::string>>> ncs;
  for (unsigned int v = 0; v < nb_messages; ++v) {
    ncs.push_back(std::make_pair(v, std::async(std::launch::async, [](std::string message, std::string address, unsigned int port) -> std::string {
#ifdef _WIN32
      std::string command = R"(python ../nc.py )" + address + " " + std::to_string(port) + R"( --timeout 10 --message ")" + message + R"(")";
#else
      std::string command = R"(echo ")" + message + R"(" | nc -G 10 )" + address + " " + std::to_string(port);
#endif
      return exec(command);
    }, get_msg(msg)(v), address, port)));
  }
  return ncs;
}

TEST_CASE("TCP Connections", "[server]") {
#if defined(_WIN32) && !defined(__INTIME__)
  WSADATA wsaData;
  WORD wVersionRequested;
  wVersionRequested = MAKEWORD(2, 2);
  WSAStartup(wVersionRequested, &wsaData);
#endif
  static std::string const address = "localhost";
  static unsigned int const port = 8888;
  static size_t const pool_size = 10;

  socketpp::Server server(8888, SOCK_STREAM, [](std::string req) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      return req;
    }, pool_size);
  server.start();

  SECTION("Handle one connection") {
    auto ncs = send_requests_and_check_result("coucou", 1, address, port);
    std::for_each(ncs.begin(), ncs.end(), [](std::pair<unsigned int, std::future<std::string>>& t) {
      CHECK(get_msg("coucou")(t.first) == t.second.get());
    });
  }

  SECTION("Handle multiple connections") {
    auto ncs = send_requests_and_check_result("coucou", pool_size, address, port);
    std::for_each(ncs.begin(), ncs.end(), [](std::pair<unsigned int, std::future<std::string>>& t) {
      CHECK(get_msg("coucou")(t.first) == t.second.get());
    });
  }

  SECTION("Handle too numerous connections") {
    size_t nb_messages = pool_size + 5;
    auto ncs = send_requests_and_check_result("coucou", nb_messages, address, port);
    auto nb_valid_result = std::accumulate(ncs.begin(), ncs.end(), 0, [](unsigned int res, std::pair<unsigned int, std::future<std::string>>& t) -> int {
      return res + ((t.second.get() == get_msg("coucou")(t.first)) ? 1 : 0);
    });
    CHECK(nb_valid_result >= pool_size);
    CHECK(nb_valid_result < nb_messages);
  }
  server.stop();

#if defined(_WIN32) && !defined(__INTIME__)
  WSACleanup();
#endif
}

#ifdef _WIN32
#undef popen
#undef pclose
#endif
