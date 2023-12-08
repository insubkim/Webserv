#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <iterator>

#include "HttpResponse.hpp"
#include "Client.hpp"

#define READ_FILE_BUF_SIZE 4096

HttpResponse::HttpResponse(const HttpRequest& req, const RouteRule& route_rule) : _http_major(1), _http_minor(1), _status(0), \
  _content_length(0), _is_chunked(false), _is_ready(false) , _is_cgi(false), _cgi_handler(req, route_rule), _request_method(HPS::kHEAD), \
  _entity_idx(0) ,_header_idx(0), _is_header_sent(false), _is_session_block(false), _is_logout_req(false), _eof(false) {
    this->initContentTypes();
    _body.reserve(RESPONSE_BUF_SIZE);
  }


void HttpResponse::initContentTypes(void) {
  static const char* extensions[] = {
    ".html", ".css", ".js", ".png", ".jpg",
    ".jpeg", ".gif", ".json", ".pdf", ".zip",
    ".tar", ".gz", ".mp4", ".mp3", ".avi",
    ".mpeg", ".wav", ".ogg", ".xml", ".txt"};
  static const char* content_types[] = {
    "text/html", "text/css", "application/javascript", "image/png", "image/jpeg",
    "image/jpeg", "image/gif", "application/json", "application/pdf", "application/zip",
    "application/x-tar", "application/gzip", "video/mp4", "audio/mpeg", "video/x-msvideo",
    "video/mpeg", "audio/x-wav", "audio/ogg", "text/xml", "text/plain"};
  for (unsigned int i = 0; i < (sizeof(extensions)/sizeof(extensions[0])); i++) {
    _contentTypes[extensions[i]] = content_types[i];
  }
}

void                                      HttpResponse::readFile(const std::string& path){
  std::ifstream i(path.c_str());
  char          buf[READ_FILE_BUF_SIZE];
  if (i.fail()) throw FileNotFoundException();
  while (true){
    i.read(buf, READ_FILE_BUF_SIZE);
    _body.insert(_body.end(), buf, buf + i.gcount());
    if (i.eof()) break ;
    if (i.fail()) throw FileNotFoundException();
    
  }
  _headers["Content-Type"] = (path.rfind('.') != std::string::npos ? _contentTypes[path.substr(path.rfind('.'))] : "text/html");
}

void                                      HttpResponse::readDir(const std::string& path){
  DIR*            dir = opendir((path).c_str());
  struct dirent*  entry;
  std::string     br = "<br />";
  if (dir == NULL)  throw FileNotFoundException();
  
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_name[0] == '.')  continue ;
    _body.insert(_body.end(), entry->d_name, entry->d_name + strlen(entry->d_name));
    _body.insert(_body.end(), br.begin(), br.end());
  }
  closedir(dir);
}

void                                      HttpResponse::deleteFile(const std::string& path){
  if (unlink(path.c_str()) != 0)  _status = 500;
}


bool                                      HttpResponse::isDir(const std::string& location){
  struct stat stat_buf;
  return stat(location.c_str(), &stat_buf) != 0 ? false :  S_ISDIR(stat_buf.st_mode);
}

void                                      HttpResponse::publish(const HttpRequest& req, const RouteRule* rule, const Client& client) {
    const std::string& location = req.getLocation();
    _request_method = req.getMethod();
    if (!rule){
      publishError(404, 0, _request_method);
      return ;
    }
    const std::string  suffix_of_location(location.substr(rule->getRoute().size(), location.size() - rule->getRoute().size()));
    _headers["Content-Type"] = "text/html";
    _headers["Connection"] = "keep-alive";
    _is_ready = true;
    try{
        if (!(rule->getAcceptedMethods() & (1 << _request_method))) {
          publishError(405, rule, _request_method);
        } else if (rule->getMaxClientBodySize() != 0 &&
                rule->getMaxClientBodySize() < req.getEntity().size()) {
          publishError(413, rule, _request_method);
        } else if (rule->getRedirection().first) {
          _status = rule->getRedirection().first;
          _headers["Location"] = rule->getRedirection().second;
        } else if (rule->getIsCgi()){
          initializeCgiProcess(req, *rule, req.getHost(), client.getPort(), client.getClientFd());
          _is_cgi = true;
          _is_ready = false;
        } else if (isDir(rule->getRoot() + suffix_of_location)) {
          _status = 200;
          if (_request_method == HPS::kDELETE){
            deleteFile(rule->getRoot() + suffix_of_location);
          } else if (rule->getIndexPage().size() &&
              (rule->getRoute() == location || rule->getRoute() + '/' == location)) {  // index page event
            if (_request_method != HPS::kHEAD)
              readFile(rule->getRoot() + "/" + rule->getIndexPage());
          } else if (rule->getAutoIndex() &&
                    (rule->getRoute() == location || rule->getRoute() + '/' == location)) {  // index page event
            if (_request_method != HPS::kHEAD)
              readDir(rule->getRoot() + suffix_of_location);
          } else{
            publishError(404, rule, _request_method);
          }
        }else{
          _status = 200;
          if (_request_method == HPS::kDELETE){
            deleteFile(rule->getRoot() + suffix_of_location);
          } else if (_request_method != HPS::kHEAD)
            readFile(rule->getRoot() + suffix_of_location);
        }
    } catch (FileNotFoundException &e){
      std::cout << "requested url not found" << std::endl;
      std::cout << e.what() << std::endl;
      publishError(404, rule, _request_method);
    }
    addContentLength();
}

void                                      HttpResponse::publishCgi(const std::vector<char>::const_iterator& begin, const std::vector<char>::const_iterator& end,  const RouteRule& rule, enum HPS::Method method) {
  std::string                       key;
  std::string                       value;

  std::vector<char>::const_iterator it = begin;
  std::vector<char>::const_iterator start = begin;
  _headers.clear();
  _headers["Content-Type"] = "text/html";
  _headers["Connection"] = "keep-alive";

  bool  is_key = true;
  while (it < end) {
    if (is_key && *it == ':'){
      while (start != it && *start == ' ') ++start;
      key = std::string(start, it);
      start = it + 1;
      is_key = false;
    } else if (!is_key && *it == '\n') {
      //[char] \r\n
      //[char] \n
      while (start != it && *start == ' ') ++start;
      if (start != it && it - begin >= 1 && *(it - 1) == '\r') { 
        value = std::string(start, it - 1);
      } else {
        value = std::string(start, it);
      }
      _headers[key] = value;
      start = it + 1;
      is_key = true;
      //[char] \r\n\r\n
      //[char] \n\n
      if (end - it >= 2 && it - begin >= 1 && *(it - 1) == '\r' && *(it + 1) == '\r' && *(it + 2) == '\n') { it += 3; break ; }
      if (end - it >= 1 && *(it + 1) == '\n') { it += 2; break ; }
    } 
    ++it;
    if (it == end) _headers.clear();
  }
  _body.insert(_body.end(), it, end);

  _is_session_block = false;
  _is_logout_req = false;
  if (_headers.find("user-name") != _headers.end()) {
    std::map<std::string, std::string>::const_iterator  iter = _headers.find("user-name");
    _session_block.setValue(iter->second);
    _headers["set-cookie"] = "session_id=" + _session_block.getId() + ";";
    _headers.erase(iter);
    _is_session_block = true;
  } else if (_headers.find("logout") != _headers.end()) {
    std::map<std::string, std::string>::const_iterator  iter = _headers.find("logout");
    _session_block.setId(iter->second);
    _headers.erase(iter);
    _is_logout_req = true;
  }
  if (_headers.find("Location") != _headers.end()) {
    std::map<std::string, std::string>::const_iterator  iter = _headers.find("Location");
    const std::string& location = iter->second;
    if (location[0] == '/'){
      _is_ready = false;
      throw LocalReDirException();
    }else{
      _status = 302;
      _status_message = "Found";
    }
  } else if (_headers.find("Content-type") != _headers.end() || _headers.find("Content-Type") != _headers.end()) {
    _status = 200;
    _status_message = "OK";
  } else {
    _status = 500;
    publishError(500, &rule, method);
  }
  _is_ready = true;
  addContentLength();
}

bool  checkErrorPage(int status, const RouteRule* rule) {
  if (!rule) return false;
  return rule->hasErrorPage(status);
}

void                                      HttpResponse::publishError(int status, const RouteRule* rule, enum HPS::Method method){
  _status = status;
  _is_ready = true;
  if (checkErrorPage(status, rule)){
    std::stringstream ss;
    ss << _status;
    if (method != HPS::kHEAD) {
      std::string body_str("<html><body><h1>" + ss.str() + " error!</h1></body></html>");
      _body.assign(body_str.begin(), body_str.end());
    }
    _headers["Content-Type"] = "text/html";
    if (status != 408)  _headers["Connection"] = "keep-alive";
    else _headers["Connection"] = "close";
    addContentLength();
    return ;
  }
  try{
    if (method != HPS::kHEAD)
      readFile(rule->getRoot() + "/" + rule->getErrorPage(status));
  } catch (FileNotFoundException &e){
    std::cout << "configured error page not found" << std::endl;
    std::cout << e.what() << std::endl;
    publishError(404, 0, method);
  }
}

void  HttpResponse::initializeCgiProcess(
  const HttpRequest& req, const RouteRule& rule, const std::string& server_name, const int& port, const int& client_fd) {
  _cgi_handler = CgiHandler(req, rule, server_name, port, client_fd);
}

int   HttpResponse::cgiExecute(const std::map<std::string, SessionBlock>::const_iterator& sbi, bool is_joined_session) {
  return _cgi_handler.execute(sbi, is_joined_session);
}

void  HttpResponse::addContentLength(void) {
  std::stringstream ss;
  ss << _body.size();
  _headers["Content-Length"] = ss.str();
  _content_length = _body.size();
}

// Getters
const unsigned short&                     HttpResponse::getHttpMajor() const { return _http_major; }
const unsigned short&                     HttpResponse::getHttpMinor() const { return _http_minor; }
const unsigned short&                     HttpResponse::getStatus() const { return _status; }
const std::string&                        HttpResponse::getStatusMessage() const { return _status_message; }
const std::map<std::string, std::string>& HttpResponse::getHeader() const { return _headers; }
const unsigned long long&                 HttpResponse::getContentLength() const { return _content_length; }
const bool&                               HttpResponse::getIsChunked() const { return _is_chunked; }
const std::vector<char>&                  HttpResponse::getBody() const { return _body; }
const bool&                               HttpResponse::getIsReady() const { return _is_ready; }
const bool&                               HttpResponse::getIsCgi() const { return _is_cgi; }
const int&                                HttpResponse::getCgiPipeIn(void) const { return _cgi_handler.getReadPipeFromCgi(); }
CgiHandler&                               HttpResponse::getCgiHandler() { return _cgi_handler; }
HPS::Method                               HttpResponse::getMethod(void) const { return _request_method; }
const int&                                HttpResponse::getEntityIdx(void) const { return _entity_idx; };
const int&                                HttpResponse::getHeaderIdx(void) const { return _header_idx; }
const bool&                               HttpResponse::getIsHeaderSent(void) const { return _is_header_sent; }
const SessionBlock&                       HttpResponse::getSessionBlock(void) const { return _session_block; }
const bool&                               HttpResponse::getIsSessionBlock(void) const { return _is_session_block; }
const bool&                               HttpResponse::getIsLogoutRequest(void) const { return _is_logout_req; }
const bool&                               HttpResponse::getEof(void) const { return _eof; }
const CgiHandler&                         HttpResponse::getCgiHandler(void) const { return _cgi_handler; }

// Setters
void                                      HttpResponse::setIsCgi(bool is_cgi) { _is_cgi = is_cgi; }
void                                      HttpResponse::setEntityIdx(int entity_idx) { _entity_idx = entity_idx; }
void                                      HttpResponse::setHeaderIdx(int header_idx) { _header_idx = header_idx; }
void                                      HttpResponse::setIsHeaderSent(bool is_header_sent) { _is_header_sent = is_header_sent; }
void                                      HttpResponse::setEof(bool eof) { _eof = eof; }

// static function
std::string                               HttpResponse::timeOutMessage() {
  static std::string message = \ 
    "HTTP/1.1 408 Request Timeout\r\n\
    Content-Type: text/html\r\n\
    Content-Length: 0\r\n\r\n";
  return message;
}