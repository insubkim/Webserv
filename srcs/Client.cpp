#include "Client.hpp"

Client::Client() : _read_idx(0), _has_eof(false), _client_fd(-1), _port(-1), _is_time_out(false), _time_out_message_idx(0) {
  _buf.reserve(CLIENT_BUF_SIZE);
}

Client::Client(int client_fd, int port, time_t last_request_time, time_t timeout_interval) 
  : _read_idx(0), _has_eof(false), _client_fd(client_fd), _port(port), _last_request_time(last_request_time),
   _timeout_interval(timeout_interval), _is_time_out(false), _time_out_message_idx(0) {
  _buf.reserve(CLIENT_BUF_SIZE);
}

Client& Client::operator=(const Client& other) {
  if (this == &other)
    return *this;
  _buf.reserve(other._buf.capacity());
  _buf = other._buf;
  _read_idx = other._read_idx;
  _reqs = other._reqs;
  _ress = other._ress;
  _has_eof = other._has_eof;
  _client_fd = other._client_fd;
  _port = other._port;
  _last_request_time = other._last_request_time;
  _timeout_interval = other._timeout_interval;
  _is_time_out = other._is_time_out;
  _time_out_message_idx = other._time_out_message_idx;
  return *this;
}

const int&                          Client::getClientFd() const { return _client_fd; }
const int&                          Client::getPort(void) const { return _port; }
const std::vector<char>&            Client::getBuf(void) const { return _buf; }
const size_t&                       Client::getReadIdx(void) const { return _read_idx; }
const std::queue<HttpRequest>&      Client::getReqs(void) const { return _reqs; }
std::queue<HttpResponse>&           Client::getRess(void) { return _ress; }
const bool&                         Client::getEof(void) const { return _has_eof; }
const time_t&                       Client::getLastRequestTime() const { return _last_request_time; }
const time_t&                       Client::getTimeoutInterval() const { return _timeout_interval; }
const bool&                         Client::getIsTimeOut(void) const { return _is_time_out; }
const int&                          Client::getTimeOutMessageIdx(void) const { return _time_out_message_idx; }

std::vector<char>::const_iterator   Client::getReadIter(void) { return _buf.begin() + _read_idx; }
std::vector<char>::const_iterator   Client::getEndIter(void) { return _buf.end(); }

HttpResponse&                       Client::getResponseByCgiFd(int fd) { return *(_http_responses_by_fd.find(fd)->second); }
HttpResponse&                       Client::getResponseByPid(int pid) { return *(_http_responses_by_pid.find(pid)->second); }

HttpResponse&                       Client::backRess(void) { return _ress.back(); }

HttpRequest&                        Client::backRequest(void) {
  return _reqs.back();
}

void                                Client::clearBuf(void) {
  _buf.clear();
  _read_idx = 0;
}

void                                Client::addBuf(const char* buf, size_t size) {
  _buf.insert(_buf.end(), buf, buf + size);
}
void                                Client::addReadIdx(size_t idx) { _read_idx += idx; }
Client&                             Client::addReqs(void) { _reqs.push(HttpRequest()); return *this; }
Client&                             Client::addRess(const HttpRequest& req, const RouteRule *route_rule) {
  if (route_rule) _ress.push(HttpResponse(req, *route_rule));
  else {
    RouteRule rule;
    _ress.push(HttpResponse(req, rule));
  }
  return *this; 
}
void                                Client::eraseBuf(void) { _buf.erase(_buf.begin(), _buf.begin() + _read_idx); _read_idx = 0; }
void                                Client::popReqs(void) { _reqs.pop(); }
void                                Client::popRess(void) { _ress.pop(); }
void                                Client::setEof(bool has_eof) { _has_eof = has_eof; }

int                                 Client::headerEndIdx(const size_t& start) {
  size_t idx = start;

  if (_buf.size() < 2) return -1;
  if (idx < 1) idx = 1;

  for (; idx < _buf.size(); ++idx) {
    if (_buf[idx] == '\n') {
      bool flag = true;
      for (int j = idx - 1; j >= 0 && j > static_cast<int>(idx) - 4; --j) {
        if (_buf[j] == '\r') {
          if (flag) flag = false;
          else break ;
        } else if (_buf[j] == '\n') return (idx + 1) - _read_idx;
        else break ;
      }
    }
  }
  return -1;
}

void  Client::subBuf(const size_t start, const size_t end, std::vector<char>& sub_buf) {
  sub_buf.insert(sub_buf.begin(), _buf.begin() + start, _buf.begin() + end);
}

void Client::setLastRequestTime(const time_t& last_request_time) {
  _last_request_time = last_request_time;
}

void Client::setTimeoutInterval(const time_t& timeout_interval) {
  _timeout_interval = timeout_interval;
}

void                                Client::setIsTimeOut(const bool& is_time_out) { _is_time_out = is_time_out; _has_eof = true; }
void                                Client::setTimeOutMessageIdx(const int& idx) { _time_out_message_idx = idx; }

void                                Client::addResponseByFd(int fd, HttpResponse* res) { _http_responses_by_fd[fd]= res; }
void                                Client::addResponseByPid(int pid, HttpResponse* res) { _http_responses_by_pid[pid] = res; }

bool                                Client::hasCgiFd(int fd) const { return _http_responses_by_fd.count(fd); }


bool                                Client::operator<(const Client& other) const {
  return this->getClientFd() < other.getClientFd();
}