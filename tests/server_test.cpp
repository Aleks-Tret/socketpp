#include <catch.hpp>

#include <socketpp/server.hpp>

#include <string>
#include <stdio.h>
#include <chrono>
#include <future>
#include <numeric>
#include <list>
#include <iostream>
#include <random>

#ifdef _WIN32
# define popen _popen
# define pclose _pclose
# include <direct.h>
#endif

std::string remove_eol(std::string && s) {
    return std::string(s.begin(), std::remove(s.begin(), s.end(), '\n'));
}

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
    return remove_eol(std::move(result));
}

std::function<std::string(unsigned int)> get_msg(std::string msg) {
  return [msg](unsigned int id) {
    return msg + "-" + std::to_string(id);
  };
}

#ifdef _WIN32
std::string get_exe_path() {
  char ownPth[MAX_PATH];
  HMODULE hModule = GetModuleHandle(nullptr);
  GetModuleFileName(hModule, ownPth, (sizeof(ownPth)));
  return std::string(ownPth);
}
#endif

unsigned int send_requests_and_check_result(std::string msg, size_t nb_messages, std::string address, unsigned int port) {
  std::list<std::pair<unsigned int, std::future<std::string>>> ncs;
  for (unsigned int v = 0; v < nb_messages; ++v) {
    ncs.push_back(std::make_pair(v, std::async(std::launch::async, [&v](std::string message, std::string address, unsigned int port) -> std::string {
#ifdef _WIN32      
      std::string command = "python3 " + get_exe_path() + R"(\..\..\nc.py )" + address + " " + std::to_string(port) + R"( --timeout 1 --message ")" + message + R"(")";
#else
      std::string command = R"(echo ")" + message + R"(" | nc -G 1 )" + address + " " + std::to_string(port);
     // std::string command = R"(python3 ~/Desktop/socketpp/tests/nc.py )" + address + " " + std::to_string(port) + R"( --timeout 1 --message ")" + message + R"(")";
#endif
      std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 50 + v*10));
      return exec(command);
    }, get_msg(msg)(v), address, port)));
  }
  return std::accumulate(ncs.begin(), ncs.end(), static_cast<unsigned int>(0), [](unsigned int res, std::pair<unsigned int, std::future<std::string>>& t) -> unsigned int {
        auto resp = t.second.get();
        auto expected = get_msg("coucou")(t.first);
    return res + static_cast<unsigned int>((resp == expected) ? 1 : 0);
  });
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
    
  std::srand(std::time(0));

  socketpp::Server server(8888, SOCK_STREAM, [](std::string req) {
      std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 5));
      return req;
    }, pool_size);
  server.start();

  SECTION("Handle one connection") {
    REQUIRE(send_requests_and_check_result("coucou", 1, address, port) == 1);
  }

  SECTION("Handle multiple connections") {
    size_t nb_messages = pool_size;
    REQUIRE(send_requests_and_check_result("coucou", nb_messages , address, port) == nb_messages);
  }

  SECTION("Handle too numerous connections") {
    size_t nb_messages = pool_size + 5;
    auto nb_valid_result = send_requests_and_check_result("coucou", nb_messages, address, port);
    REQUIRE(nb_valid_result >= pool_size);
    REQUIRE(nb_valid_result <= nb_messages);
  }
#if defined(_WIN32) && !defined(__INTIME__)
  WSACleanup();
#endif
}

#ifdef _WIN32
#undef popen
#undef pclose
#endif
