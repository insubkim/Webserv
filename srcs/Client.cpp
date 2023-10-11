#include "Client.hpp"

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
  if (start < 3) return -1;

  for (size_t  idx = start; idx < _buf.size(); ++idx) {
    if (_buf[idx - 3] == '\r' && _buf[idx - 2] == '\n' && \
        _buf[idx - 1] == '\r' && _buf[idx] == '\n')
      return idx;
  }
  return -1;
}

const std::string                 Client::subBuf(const size_t start, const size_t end) {
  std::vector<char>::iterator sit = _buf.begin();
  std::vector<char>::iterator eit = _buf.end();
  std::advance(sit, start);
  std::advance(eit, -end);
  return std::string(sit, eit);
}