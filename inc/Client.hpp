#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class Client {
  private:
    std::vector<char>                 _buf;
    size_t                            _read_idx;

    std::vector<HttpRequest>          _reqs;
    std::vector<HttpResponse>         _ress;

    bool                              _has_eof;


  public:
    const std::vector<char>&          getBuf(void) const;
    const size_t&                     getReadIdx(void) const;
    const std::vector<HttpRequest>&   getReqs(void) const;
    const std::vector<HttpResponse>&  getRess(void) const;
    const bool&                       getHasEof(void) const;

    void                              clearBuf(void);

    void                              addBuf(const char* buf, size_t size);
    void                              addReadIdx(const size_t idx);
    void                              addReqs(const HttpRequest& req);
    void                              addRess(const HttpResponse& res);

    void                              clearRess(void);
    void                              setHasEof(const bool has_eof);

    int                               headerEndIdx(const size_t& start);
    const std::string                 subBuf(const size_t start, const size_t end);
};

#endif