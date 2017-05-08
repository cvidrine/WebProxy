/**
 * File: request-handler.h
 * -----------------------
 * Defines the HTTPRequestHandler class, which fully proxies and
 * services a single client request.  
 */

#ifndef _request_handler_
#define _request_handler_

#include <utility>
#include <string>
#include "request.h"
#include <socket++/sockstream.h> 
#include "blacklist.h"
#include "cache.h"

class HTTPRequestHandler {
 public:
  void serviceRequest(const std::pair<int, std::string>& connection, const std::pair<bool, std::string>& serverPair, unsigned short portNum) throw();
  void clearCache();
  void setCacheMaxAge(long maxAge);

 private: 
  HTTPBlacklist blacklist;
  HTTPCache cache = HTTPCache();
  bool blackListed(const std::string& server){ return !blacklist.serverIsAllowed(server); }
  HTTPRequest createRequest(iosockstream& ss, const std::string& clientIPAddress);
  HTTPResponse getResponse(HTTPRequest request, const std::pair<bool, std::string>& serverPair, unsigned short portNum);
  std::mutex mutexArr[997];
  bool initialized = false;
};

#endif
