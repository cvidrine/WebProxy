/**
 * File: request-handler.cc
 * ------------------------
 * Provides the implementation for the HTTPRequestHandler class.
 */

#include "request-handler.h"
#include "client-socket.h"
#include "request.h"
#include "response.h"
#include "ostreamlock.h"
// for sockbuf, iosockstream
using namespace std;

static const string black_list_file = "blocked-domains.txt";

void sendLoopResponse(iosockstream& ss){
    HTTPResponse response;
    response.setResponseCode(504);
    response.setPayload("proxy chain contains loop.");
    ss << response;
    ss.flush();
}

HTTPRequest HTTPRequestHandler::createRequest(iosockstream& ss, const string& clientIPAddress){
  HTTPRequest request;
  try{
  request.ingestRequestLine(ss);
  request.ingestHeader(ss, clientIPAddress);
  request.ingestPayload(ss);
  } catch(const HTTPBadRequestException &err){
      cout << "Recieved bad request from client." << endl;
      return request;
  }
  return request;
}

void sendBlacklistResponse(iosockstream& ss){
    HTTPResponse response;
    response.setResponseCode(403);
    response.setPayload("Forbidden Content");
    ss << response;
    ss.flush();
}


void printRequest(HTTPRequest request){
  cout << oslock << request.getMethod() <<" "<< request.getPath() << " " << request.getProtocol() << endl <<osunlock;
}

HTTPResponse HTTPRequestHandler::getResponse(HTTPRequest request, const pair<bool, string>& serverPair, unsigned short portNum){
    int index = cache.getMutexIndex(request);
    mutexArr[index].lock();
    HTTPResponse response;
    try{
        if(cache.containsCacheEntry(request, response)) {
            mutexArr[index].unlock();
            return response;
        }
    }catch(const HTTPCacheAccessException& hce){
        cout << "Problem accessing cache."  << endl;
    }
    
    string server;
    unsigned short port;
    if(serverPair.first){
        server = serverPair.second;
        port = portNum;
    } else{
        server = request.getServer();
        port = request.getPort();
    }

    //Create connection to origin server.
    sockbuf client_sb(createClientSocket(server, port));
    iosockstream client_ss(&client_sb);

    //Sends request to origin server
    client_ss << request;
    client_ss.flush();

    //Create and return response
    response.ingestResponseHeader(client_ss);
    if(request.getMethod() != "HEAD") response.ingestPayload(client_ss);
    if(!cache.containsCacheEntry(request, response) && cache.shouldCache(request, response)) {
        try{
          cache.cacheEntry(request, response);
        } catch(const HTTPCacheAccessException& hce){
            cout << "Problem adding entry to cache." <<endl;
        }
     }
    mutexArr[index].unlock();
    cout << "returning response" <<endl;
    return response;
}




void HTTPRequestHandler::serviceRequest(const pair<int, string>& connection, const pair<bool, string>& serverPair, unsigned short portNum) throw() {
  if(!initialized){
      blacklist.addToBlacklist(black_list_file);
      initialized = true;
  }
  sockbuf sb(connection.first);
  iosockstream ss(&sb);
  HTTPRequest request = createRequest(ss, connection.second);

  if(request.getLoop()){
      sendLoopResponse(ss);
      return;
  }

  if(blackListed(request.getServer())){
      sendBlacklistResponse(ss);
      return;
  }

  printRequest(request);
  request.setChain(serverPair.first);
  HTTPResponse response = getResponse(request, serverPair, portNum);
  ss << response;
  ss.flush();
}

// the following two methods needs to be completed 
// once you incorporate your HTTPCache into your HTTPRequestHandler
void HTTPRequestHandler::clearCache() { cache.clear();}
void HTTPRequestHandler::setCacheMaxAge(long maxAge) { cache.setMaxAge(maxAge);}
