#include <catch.hpp>

#include <string>
#include <stdio.h>
#include <chrono>
#include <future>
#include <numeric>
#include <list>
#include <iostream>

#ifdef _WIN32
# define popen _popen
# define pclose _pclose
# include <direct.h>
#endif

#include <socketpp/server.hpp>

std::string remove_eol(std::string && s) {
    return std::string(s.begin(), std::remove(s.begin(), s.end(), '\n'));
}

void FILE_deleter(FILE* f) {
  pclose(f);
}

std::string exec(std::string cmd) {
  std::unique_ptr<FILE, decltype(&FILE_deleter)> pipe(popen(cmd.c_str(), "r"), &FILE_deleter);
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while(!feof(pipe.get())) {
    if(fgets(buffer, 128, pipe.get()) != nullptr)
      result += buffer;
  }
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
      std::string command = "python " + get_exe_path() + R"(\..\..\nc.py )" + address + " " + std::to_string(port) + R"( --timeout 1 --message ")" + message + R"(")";
#else
      std::string command = R"(echo ")" + message + R"(" | nc )" + address + " " + std::to_string(port);
//      std::string command = R"(/usr/local/bin/python3 ~/Desktop/socketpp/tests/nc.py )" + address + " " + std::to_string(port) + R"( --timeout 1 --message ")" + message + R"(")";
#endif
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

  socketpp::Server* server = new socketpp::Server(port, SOCK_STREAM, [](std::string req) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      return req;
    });
  server->start();

  SECTION("Handle one connection") {
    CHECK(send_requests_and_check_result("coucou", 1, address, port) == 1);
  }

  SECTION("Handle too numerous connections") {
    size_t nb_messages = 20;
    auto nb_valid_result = send_requests_and_check_result("coucou", nb_messages, address, port);
    CHECK(nb_valid_result > 15);
  }
  delete server;
    
#if defined(_WIN32) && !defined(__INTIME__)
  WSACleanup();
#endif
}

#ifdef _WIN32
#undef popen
#undef pclose
#endif
