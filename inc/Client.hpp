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

    int                               _port;
    time_t                            _last_request_time;
    time_t                            _timeout_interval;
  public:
    Client();
    Client(int port, time_t last_request_time, time_t timeout_interval);

    Client& operator=(const Client& other);

    const std::vector<char>&          getBuf(void) const;
    const size_t&                     getReadIdx(void) const;
    const std::vector<HttpRequest>&   getReqs(void) const;
    const std::vector<HttpResponse>&  getRess(void) const;
    const bool&                       getHasEof(void) const;
    const int&                        getPort(void) const;
    const time_t&                     getLastRequestTime() const;
    const time_t&                     getTimeoutInterval() const;

    HttpRequest&                      lastRequest(void);

    void                              clearBuf(void);

    void                              addBuf(const char* buf, size_t size);
    void                              addReadIdx(size_t idx);
    void                              addReqs(HttpRequest& req);
    void                              addRess(HttpResponse& res);

    void                              clearRess(void);
    void                              setHasEof(bool has_eof);

    int                               headerEndIdx(const size_t& start);
    const std::vector<char>           subBuf(const size_t start, const size_t end);

    void                              setLastRequestTime(const time_t& last_request_time);
    void                              setTimeoutInterval(const time_t& timeout_interval);
};

#endif