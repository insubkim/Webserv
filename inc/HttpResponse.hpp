#ifndef HTTPRESPONSE_HPP_
#define HTTPRESPONSE_HPP_

#include "HttpDecoder.hpp"
#include "HttpRequest.hpp"
#include "RouteRule.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#define BUF_SIZE 4096

class HttpResponse {
  private:

    // start line
    unsigned short                            _http_major;
    unsigned short                            _http_minor;

    unsigned short                            _status;
    std::string                               _status_message;

    // headers
    std::map<std::string, std::string>        _headers;
    unsigned long long                        _content_length;
    bool                                      _is_chunked;

    // body
    std::vector<char>                         _body;

    void                                      readFile(const std::string& path);
    void                                      readDir(const std::string& path);

  public:
    HttpResponse();

    const unsigned short&                     getHttpMajor(void) const;
    const unsigned short&                     getHttpMinor(void) const;

    const unsigned short&                     getStatus(void) const;
    const std::string&                        getStatusMessage(void) const;

    const std::map<std::string, std::string>& getHeader(void) const;
    const unsigned long long&                 getContentLength(void) const;
    const bool&                               getIsChunked(void) const;
    const std::vector<char>&                  getBody(void) const;

    void                                      setHttpMajor(unsigned short http_major);
    void                                      setHttpMinor(unsigned short http_minor);
    void                                      setStatus(unsigned short status);
    void                                      setStatusMessage(const std::string& status_message);
    void                                      setHeaders(const std::map<std::string, std::string>& headers);
    void                                      setContentLength(unsigned long long content_length);
    void                                      setIsChunked(bool is_chunked);
    void                                      setBody(const std::vector<char>& body);

    void                                      addContentLength(void);
    void                                      publish(const HttpRequest& req, const RouteRule& r);
    void                                      publicError(int status);
    void                                      setHeader(const std::string& key, const std::string& value); 
};

#endif