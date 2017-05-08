/**
 * File: scheduler.cc
 * ------------------
 * Presents the implementation of the HTTPProxyScheduler class.
 */

#include "scheduler.h"
#include <utility>
using namespace std;

static const int kNumThreads = 64;

HTTPProxyScheduler::HTTPProxyScheduler() : requestPool(kNumThreads){}


void HTTPProxyScheduler::setProxyServer(string proxyServString){
    usingProxy = true;
    proxyServer = proxyServString;
}

void HTTPProxyScheduler::scheduleRequest(int clientfd, const string& clientIPAddress) throw () {
  const pair<int, string> currPair = make_pair(clientfd, clientIPAddress);
  const pair<bool, string> serverPair = make_pair(usingProxy, proxyServer);
  unsigned short port_num = proxyPortNum;
  requestPool.schedule([this, currPair, serverPair, port_num]{
          requestHandler.serviceRequest(currPair, serverPair, port_num);});

}

