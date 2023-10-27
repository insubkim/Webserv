#include "Client.hpp"

Client::Client() : _read_idx(0), _port(-1) {}

Client::Client(int port, time_t last_request_time, time_t timeout_interval) 
  : _read_idx(0), _port(port), _last_request_time(last_request_time), _timeout_interval(timeout_interval) {}

Client& Client::operator=(const Client& other) {
  if (this == &other)
    return *this;
  _buf = other._buf;
  _read_idx = other._read_idx;
  _reqs = other._reqs;
  _ress = other._ress;
  _has_eof = other._has_eof;
  _port = other._port;
  _last_request_time = other._last_request_time;
  _timeout_interval = other._timeout_interval;
  return *this;
}

const int&                        Client::getPort(void) const { return _port; }
const std::vector<char>&          Client::getBuf(void) const { return _buf; }
const size_t&                     Client::getReadIdx(void) const { return _read_idx; }
const std::vector<HttpRequest>&   Client::getReqs(void) const { return _reqs; }
const std::vector<HttpResponse>&  Client::getRess(void) const { return _ress; }
const bool&                       Client::getHasEof(void) const { return _has_eof; }
const time_t&                     Client::getLastRequestTime() const { return _last_request_time; }
const time_t&                     Client::getTimeoutInterval() const { return _timeout_interval; }

HttpRequest&                      Client::lastRequest(void) {
  std::vector<HttpRequest>::iterator  it = _reqs.end(); --it;
  return *it;//_reqs.end()--; 해도될거같아요.
}

void                              Client::clearBuf(void) {
  _buf.clear();
  _read_idx = 0;
}

void                              Client::addBuf(const char* buf, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    _buf.push_back(buf[i]);//insert 로 바꾸는게 빠를거같아요.
  }
}
void                              Client::addReadIdx(size_t idx) { _read_idx += idx; }
void                              Client::addReqs(HttpRequest& req) { _reqs.push_back(req); }
void                              Client::addRess(HttpResponse& res) { _ress.push_back(res); }
void                              Client::clearRess(void) {_ress.clear(); }
void                              Client::setHasEof(bool has_eof) { _has_eof = has_eof; }

int                               Client::headerEndIdx(const size_t& start) {
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
        } else if (_buf[j] == '\n') return idx + 1;
        else break ;
      }
    }
  }
  return -1;
}

const std::vector<char>  Client::subBuf(const size_t start, const size_t end) {
  std::vector<char> sub_buf;

  for (size_t i = start;i < end; ++i) {
    sub_buf.push_back(_buf[i]);//insert가 더 빠를거같아요. 
  }
  return sub_buf;
}

void Client::setLastRequestTime(const time_t& last_request_time) {
  _last_request_time = last_request_time;
}

void Client::setTimeoutInterval(const time_t& timeout_interval) {
  _timeout_interval = timeout_interval;
}
