#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <sstream>
#include "CgiHandler.hpp"
#include <unistd.h>
#include <iostream>


CgiHandler::CgiHandler() {}

CgiHandler::CgiHandler(
    const HttpRequest& req, const RouteRule& route_rule, const std::string& server_name, const int& port, const int& client_fd) throw(std::runtime_error)
  : _idx(0), _req(req), _route_rule(route_rule), _server_name(server_name), _port(port), _client_fd(client_fd) {
  this->setPipe();
}

CgiHandler::CgiHandler(const CgiHandler& other)
  : _idx(other._idx), _req(other._req), _route_rule(other._route_rule) {
  _pipe_from_cgi_fd[PIPE_READ] = other._pipe_from_cgi_fd[PIPE_READ];
  _pipe_from_cgi_fd[PIPE_WRITE] = other._pipe_from_cgi_fd[PIPE_WRITE];
  _pipe_to_cgi_fd[PIPE_READ] = other._pipe_to_cgi_fd[PIPE_READ];
  _pipe_to_cgi_fd[PIPE_WRITE] = other._pipe_to_cgi_fd[PIPE_WRITE];
  _server_name = other._server_name;
  _port = other._port;
  _buf = other._buf;
  _client_fd = other._client_fd;
}

CgiHandler& CgiHandler::operator=(const CgiHandler& other) {
  if (this == & other)
    return *this;
  _idx = other._idx;
  _req = other._req;
  _route_rule = other._route_rule;
  _pipe_from_cgi_fd[PIPE_READ] = other._pipe_from_cgi_fd[PIPE_READ];
  _pipe_from_cgi_fd[PIPE_WRITE] = other._pipe_from_cgi_fd[PIPE_WRITE];
  _pipe_to_cgi_fd[PIPE_READ] = other._pipe_to_cgi_fd[PIPE_READ];
  _pipe_to_cgi_fd[PIPE_WRITE] = other._pipe_to_cgi_fd[PIPE_WRITE];
  _server_name = other._server_name;
  _port = other._port;
  _buf = other._buf;
  _client_fd = other._client_fd;
  return *this;
}

int CgiHandler::getCgiReqEntityIdx(void) { return _idx; }
HttpRequest& CgiHandler::getRequest(void) { return _req; }
const int& CgiHandler::getReadPipeFromCgi(void) const { return _pipe_from_cgi_fd[PIPE_READ]; }
const int& CgiHandler::getWritePipetoCgi(void) const { return _pipe_to_cgi_fd[PIPE_WRITE]; }

void CgiHandler::setCgiReqEntityIdx(int idx) { _idx = idx; }
void CgiHandler::setPipe(void) throw(std::runtime_error) {
  if (pipe(_pipe_from_cgi_fd) != 0) throw std::runtime_error("error: can't open pipe");
  fcntl(_pipe_from_cgi_fd[PIPE_READ], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
  fcntl(_pipe_from_cgi_fd[PIPE_WRITE], F_SETFL, O_NONBLOCK, FD_CLOEXEC);


  if (pipe(_pipe_to_cgi_fd) != 0) throw std::runtime_error("error: can't open pipe");
  fcntl(_pipe_to_cgi_fd[PIPE_READ], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
  fcntl(_pipe_to_cgi_fd[PIPE_WRITE], F_SETFL, O_NONBLOCK, FD_CLOEXEC);

}

void CgiHandler::setupCgiArgv(void){
  _argv[0] = strdup(_route_rule.getCgiPath().c_str());
  _argv[1] = strdup((_route_rule.getRoot() + _req.getLocation()).c_str());
  _argv[2] = NULL;
  if (errno == ENOMEM) throw std::runtime_error("error: can't allocate memory");
}


void CgiHandler::setupCgiEnvp(void) {
  static const char* methods[] = {GET, HEAD, POST, DELETE};
  std::stringstream ss;

  setenv("SERVER_SOFTWARE", "webserv/1.0", 1);
  _envp[0] = strdup("SERVER_SOFTWARE=webserv/1.0");
  if (_req.getEntityArrived()) {
    ss << _req.getEntity().size();
    setenv("CONTENT_LENGTH", ss.str().c_str(), 1);
    _envp[1] = strdup(("CONTENT_LENGTH=" + ss.str()).c_str());
  } else{
    setenv("CONTENT_LENGTH", "0", 1);
    _envp[1] = strdup("CONTENT_LENGTH=0");
  }
  std::string content_type = _req.getHeaderValue("content-type");
  setenv("CONTENT_TYPE", (content_type.empty() ? DEFAULT_CONTENT_TYPE : content_type.c_str()), 1);
  _envp[2] = strdup(("CONTENT_TYPE=" + (content_type.empty() ? DEFAULT_CONTENT_TYPE : content_type)).c_str());
  setenv("REDIRECT_STATUS", "CGI", 1);
  _envp[3] = strdup("REDIRECT_STATUS=CGI");
  setenv("GATEWAY_INTERFACE",  "CGI/1.1", 1);
  _envp[4] = strdup("GATEWAY_INTERFACE=CGI/1.1");
  setenv("REMOTE_ADDR", "127.0.0.1", 1);
  _envp[5] = strdup("REMOTE_ADDR=127.0.0.1");
  setenv("REQUEST_METHOD", methods[_req.getMethod() - 1], 1);
  _envp[6] = strdup(("REQUEST_METHOD=" + std::string(methods[_req.getMethod() - 1])).c_str());
  setenv("QUERY_STRING", _req.getQueries().c_str(), 1);
  _envp[7] = strdup(("QUERY_STRING=" + _req.getQueries()).c_str());
  setenv("SCRIPT_NAME", (_route_rule.getRoot() + _req.getLocation()).c_str(), 1);
  _envp[8] = strdup(("SCRIPT_NAME=" + _route_rule.getRoot() + _req.getLocation()).c_str());
  setenv("PATH_INFO", (_route_rule.getRoot() + _req.getLocation()).c_str(), 1);
  _envp[9] = strdup(("PATH_INFO=" + _route_rule.getRoot() + _req.getLocation()).c_str());
  
  char current_dir[512];
  getcwd(current_dir, 512);
  std::string slash = "/";
  setenv("PATH_TRANSLATED", (current_dir + slash + _route_rule.getRoot() + _req.getLocation()).c_str(), 1);
  _envp[10] = strdup(("PATH_TRANSLATED=" + std::string(current_dir) + slash + _route_rule.getRoot() + _req.getLocation()).c_str());
  setenv("REQUEST_URI", (_route_rule.getRoot() + _req.getLocation() + "?" + _req.getQueries()).c_str(), 1);
  _envp[11] = strdup(("REQUEST_URI=" + _route_rule.getRoot() + _req.getLocation() + "?" + _req.getQueries()).c_str());
  setenv("SERVER_NAME", _server_name.c_str(), 1);
  _envp[12] = strdup(("SERVER_NAME=" + _server_name).c_str());
  ss << _port;
  setenv("SERVER_PORT", ss.str().c_str(), 1);
  _envp[13] = strdup(("SERVER_PORT=" + ss.str()).c_str());
  setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
  _envp[14] = strdup("SERVER_PROTOCOL=HTTP/1.1");

  if (!_req.getHeaderValue("x-secret-header-for-test").empty()){
    setenv("HTTP_X_SECRET_HEADER_FOR_TEST", _req.getHeaderValue("x-secret-header-for-test").c_str(), 1);
    _envp[15] = strdup(("HTTP_X_SECRET_HEADER_FOR_TEST=" + _req.getHeaderValue("x-secret-header-for-test")).c_str());
  }
  if (errno == ENOMEM) throw std::runtime_error("error: can't allocate memory");
}
int CgiHandler::execute(void) throw(std::runtime_error) {
  pid_t pid = fork();
  if (pid == -1) throw std::runtime_error("error: fork error!");
  if (pid == 0) {  // Child process
      close(_pipe_from_cgi_fd[PIPE_READ]);
      dup2(_pipe_from_cgi_fd[PIPE_WRITE], STDOUT_FILENO);
      close(_pipe_from_cgi_fd[PIPE_WRITE]);

      close(_pipe_to_cgi_fd[PIPE_WRITE]);
      dup2(_pipe_to_cgi_fd[PIPE_READ], STDIN_FILENO);
      close(_pipe_to_cgi_fd[PIPE_READ]);

      this->setupCgiEnvp();
      this->setupCgiArgv();

      if (execve(_route_rule.getCgiPath().c_str(), _argv, _envp) == -1) {
        exit(EXIT_FAILURE);
      }
      return 0;  //  This part is actually not reached.
  } else {  // Parent process
      close(_pipe_from_cgi_fd[PIPE_WRITE]);
      close(_pipe_to_cgi_fd[PIPE_READ]);
  }
  return pid;
}

const int&                CgiHandler::getClientFd(void) const{ return _client_fd; }
const std::vector<char>&  CgiHandler::getBuf(void) const{ return _buf; }
const RouteRule&          CgiHandler::getRouteRule(void) const { return _route_rule; };

void  CgiHandler::addBuf(const char* buf, size_t size){ _buf.insert(_buf.end(), buf, buf + size); }
void  CgiHandler::closeReadPipe(void){ close(_pipe_from_cgi_fd[PIPE_READ]); }
