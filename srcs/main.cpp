#include "Server.hpp"

int main(int argc, char** argv) {
  if (argc > 2) {
    std::cout << "usage: ./webserv [config_file]" << std::endl;
    return 1;
  }

  Server  server(argc == 2 ? argv[1] : "./utils/webserv.config");
  server.init();
  server.run();
  return 0;
}