#include "Client.hpp"

Client::Client() : _read_idx(0) {}

const std::vector<char>&          Client::getBuf(void) const { return _buf; }
const size_t&                     Client::getReadIdx(void) const { return _read_idx; }
const std::vector<HttpRequest>&   Client::getReqs(void) const { return _reqs; }
const std::vector<HttpResponse>&  Client::getRess(void) const { return _ress; }
const bool&                       Client::getHasEof(void) const { return _has_eof; }

void                              Client::clearBuf(void) {
  _buf.clear();
  _read_idx = 0;
}

void                              Client::addBuf(const char* buf, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    _buf.push_back(buf[i]);
  }
}
void                              Client::addReadIdx(const size_t idx) { _read_idx += idx; }
void                              Client::addReqs(const HttpRequest& req) { _reqs.push_back(req); }
void                              Client::addRess(const HttpResponse& res) { _ress.push_back(res); }
void                              Client::clearRess(void) {_ress.clear(); }
void                              Client::setHasEof(const bool has_eof) { _has_eof = has_eof; }

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

const std::string                 Client::subBuf(const size_t start, const size_t end) {
  std::vector<char>::iterator sit = _buf.begin();
  std::vector<char>::iterator eit = _buf.begin();
  std::advance(sit, start);
  std::advance(eit, end);
  return std::string(sit, eit);
}