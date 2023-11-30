#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <map>
#include <vector>
#include <set>

#include "Client.hpp"
#include "Host.hpp"
#include "CgiHandler.hpp"
#include "SessionBlock.hpp"

#define BACKLOG 512
#define EVENT_LIST_SIZE 512
#define KEEPALIVETIMEOUT 6000
#define SESSIONTIMELIMIT 1200

#define BUF_SIZE 1000000

class Server {
 private:

  enum Event {
    kEventConnectNewClient = 0,
    kEventClientClosedSocket,
    kEventReadClientRequest,
    kEventWriteClientRequest,
    kEventReadCgiResponse,
    kEventWriteCgiRequest,
    kEventCgiExited,
    kEventCheckTime,
    kEventError,
    kEventIgnore,
  };

  Host                                        _default_host;
  std::map<std::pair<std::string, int>, Host> _hosts;

  std::map<int, int>                          _server_sockets;  // <socket_fd, port>


  int                                         _kq;
  std::vector<struct kevent>                  _change_list;
  std::map<std::string, SessionBlock>         _session_blocks;

  std::set<Client>                            _clients;
  std::set<Client*>                           _clients_address;

  char                                        _buf[BUF_SIZE];

  void              handleErrorKevent(int ident, void *udata);
  void              connectClient(int server_socket);
  void              disconnectClient(Client* client);
  void              recvHttpRequest(int client_fd, Client& client, int64_t event_size);
  void              sendHttpResponse(int client_fd, Client& client, int64_t event_size);
  void              recvCgiResponse(int cgi_fd, Client& client, int64_t event_size);
  void              sendCgiRequest(int cgi_fd, Client& client, int64_t event_size);
  void              checkTimeout(void);

  int               identfyEvent(struct kevent& curr_event) const;

  bool              isClientEvent(struct kevent& curr_event) const;
  bool              isServerEvent(struct kevent& curr_event) const;
  bool              isCgiEvent(struct kevent& curr_event) const;

  void              handleConnectNewClient(struct kevent& curr_event);
  void              handleClientClosedSocket(struct kevent& curr_event);
  void              handleReadClientRequest(struct kevent& curr_event);
  void              handleWriteClientRequest(struct kevent& curr_event);
  void              handleReadCgiResponse(struct kevent& curr_event);
  void              handleWriteCgiRequest(struct kevent& curr_event);
  void              handleCgiExited(struct kevent& curr_event);
  void              handleCheckTime(struct kevent& curr_event);
  void              handleError(struct kevent& curr_event);


  void              setSocketOption(int socket_fd);

  void              changeEvents(std::vector<struct kevent> &change_list, uintptr_t ident,
                     int16_t filter, uint16_t flags, uint32_t fflags,
                     intptr_t data, void *udata);

  void              setCgiSetting(HttpResponse& res, Client& client, const std::map<std::string, SessionBlock>::const_iterator& sbi, bool is_joined_session); 
  const RouteRule*  findRouteRule(const HttpRequest& req, const int& host_port);

  time_t            getTime(void);  //return seconds
  
 public:
  Server(const char *configure_file);
  ~Server();

  void init(void);
  void run(void);

  bool                                                      isJoinedSession(const std::string& session_id);
  const std::map<std::string, SessionBlock>::const_iterator getSessionBlock(const std::string& session_id);

  template <typename T>
  T min(T first, T second) {
    return first < second ? first : second;
  }

};

#endif
