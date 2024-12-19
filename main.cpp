#include "gen-cpp/ogon.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

#include <PCSC/winscard.h>
#include <mutex>
#include <iostream>

class ogonHandler : virtual public ogonIf {

  // This is code for current project only, it don't need into ogon
  // ------------- Begin -------------
  std::mutex mtx_;
  std::map<SCARDHANDLE, SCARDCONTEXT> Card2Context_;
  // -------------  End  -------------

public:
  ogonHandler() {
    // Your initialization goes here
  }

  void EstablishContext(return_ec& _return, const DWORD_RPC dwScope) {

   	SCARDCONTEXT hContext;

    printf ("Server received SCardEstablishContext dwScope: %ld\n", dwScope);

    LONG rv = SCardEstablishContext(dwScope, NULL, NULL, &hContext);

    printf ("SCardEstablishContext return %ld, Server send hContext: %ld\n", rv, hContext);

    _return.cardContext = hContext;
    _return.retValue = rv;
  }

  LONG_RPC ReleaseContext(const SCARDCONTEXT_RPC hContext) {
    // Your implementation goes here
    printf("ReleaseContext\n");

    printf ("Server received SCardReleaseContext dwScope: %ld\n", hContext);

    LONG rv = SCardReleaseContext(hContext);

    printf ("SCardReleaseContext return %ld\n", rv);

    return rv;
  }

  void ListReaders(return_lr& _return, const SCARDCONTEXT_RPC hContext, const DWORD_RPC pcchReaders) {
    // Your implementation goes here

    LPSTR szReaderName = NULL;
    DWORD szReaderNameLen = pcchReaders;

    std::string readerBuf;

    printf ("Server received SCardListReaders: SCARDCONTEXT=%ld\n", hContext);

    if(SCARD_AUTOALLOCATE != szReaderNameLen) {
      readerBuf.resize(szReaderNameLen);
      szReaderName = readerBuf.data();
    }

    LONG rv = SCardListReaders(hContext, NULL, (readerBuf.empty() ? (LPSTR)&szReaderName : szReaderName), &szReaderNameLen);

    printf ("SCardListReaders return %ld, Server send list readers=%s\n", rv, szReaderName);

    _return.retValue = rv;
    _return.mszReaders = std::string(szReaderName, szReaderNameLen);

    if(SCARD_AUTOALLOCATE == pcchReaders) {
      SCardFreeMemory(hContext, szReaderName);
    }
  }

  void ListReaderGroups(return_lrg& _return, const SCARDCONTEXT_RPC hContext, const DWORD_RPC pcchGroups) {
    // Your implementation goes here
    LPSTR szGroups = NULL;
    DWORD szGroupsNameLen = pcchGroups;

    std::string readerBuf;

    if(SCARD_AUTOALLOCATE != szGroupsNameLen) {
      readerBuf.resize(szGroupsNameLen);
      szGroups = readerBuf.data();
    }

    printf ("Server received SCardListReaderGroups: SCARDCONTEXT=%ld\n", hContext);

    LONG rv = SCardListReaderGroups(hContext, (readerBuf.empty() ? (LPSTR)&szGroups : szGroups), &szGroupsNameLen);

    printf ("SCardListReaderGroups return %ld, Server send list groups=%s\n", rv, szGroups);

    _return.retValue = rv;
    _return.mszGroups = std::string(szGroups, szGroupsNameLen);

    if(SCARD_AUTOALLOCATE == pcchGroups) {
      SCardFreeMemory(hContext, szGroups);
    }
  }

  void Connect(return_c& _return, const SCARDCONTEXT_RPC hContext, const LPCSTR_RPC& szReader, const DWORD_RPC dwShareMode, const DWORD_RPC dwPreferredProtocols) {

    // Your implementation goes here
    SCARDHANDLE phCard;
    DWORD pdwActiveProtocol;

    printf ("Server received SCardConnect: SCARDCONTEXT=%ld\n", hContext);

    LONG rv = SCardConnect(hContext, szReader.c_str(), dwShareMode, dwPreferredProtocols, &phCard, &pdwActiveProtocol);

    printf ("SCardConnect return %ld, Server send SCARDHANDLE=%ld\n", rv, phCard);

    _return.retValue = rv;
    _return.phCard = phCard;
    _return.pdwActiveProtocol = pdwActiveProtocol;

    // This is code for current project only, it don't need into ogon
    // ------------- Begin -------------
    std::lock_guard<std::mutex> lock(mtx_);
    Card2Context_[phCard] = hContext;
    // -------------  End  -------------
  }

  void Reconnect(return_r& _return, const SCARDHANDLE_RPC hCard, const DWORD_RPC dwShareMode, const DWORD_RPC dwPreferredProtocols, const DWORD_RPC dwInitialization) {
    // Your implementation goes here
    printf("Reconnect\n");
    DWORD pdwActiveProtocol;

    printf ("Server received SCardReconnect: SCARDHANDLE=%ld\n", hCard);

    LONG rv = SCardReconnect(hCard, dwShareMode, dwPreferredProtocols, dwInitialization, &pdwActiveProtocol);

    printf ("SCardReconnect return %ld, Server send pdwActiveProtocol=%ld\n", rv, pdwActiveProtocol);

    _return.retValue = rv;
    _return.pdwActiveProtocol = pdwActiveProtocol;
  }

  LONG_RPC Disconnect(const SCARDHANDLE_RPC hCard, const DWORD_RPC dwDisposition) {
    // Your implementation goes here
    printf("Disconnect\n");

    printf ("Server received SCardDisconnect: SCARDHANDLE=%ld\n", hCard);

    LONG rv = SCardDisconnect(hCard, dwDisposition);

    printf ("SCardDisconnect return %ld\n", rv);

    // This is code for current project only, it don't need into ogon
    // ------------- Begin -------------
    std::lock_guard<std::mutex> lock(mtx_);
    Card2Context_.erase(hCard);
    // -------------  End  -------------
    
    return rv;
  }

  void Status(return_s& _return, const SCARDHANDLE_RPC hCard, const DWORD_RPC pcchReaderLen, const DWORD_RPC pcbAtrLen) {
    // Your implementation goes here

    DWORD pdwState;
    DWORD pdwProtocol;

    LPSTR szReaderName = NULL;
    DWORD szReaderNameLen = pcchReaderLen;

    std::string readerBuf;

    if(SCARD_AUTOALLOCATE != szReaderNameLen) {
      readerBuf.resize(szReaderNameLen);
      szReaderName = readerBuf.data();
    }

    LPBYTE pAtr = NULL;
    DWORD AtrLen = pcbAtrLen;

    std::string atrBuf;

    if(SCARD_AUTOALLOCATE != AtrLen) {
      atrBuf.resize(AtrLen);
      pAtr = (unsigned char*)atrBuf.data();
    }

    printf("Server received SCardStatus: SCARDHANDLE=%ld\n", hCard);

    LONG rv = SCardStatus(hCard, (readerBuf.empty() ? (LPSTR)&szReaderName : szReaderName), &szReaderNameLen, 
                                  &pdwState, &pdwProtocol, 
                                 (atrBuf.empty() ? (LPBYTE)&pAtr : pAtr), &AtrLen);

    _return.retValue = rv;
    _return.szReaderName = std::string(szReaderName, szReaderNameLen);
    _return.pdwState = pdwState;
    _return.pdwProtocol = pdwProtocol;
    _return.pbAtr = std::string((char*)pAtr, AtrLen);

    printf ("SCardStatus AtrLen=%ld ", AtrLen);
    
    for (DWORD i=0; i<AtrLen; i++)
		  printf(" %02X", pAtr[i]);

    printf ("SCardStatus return %ld\n", rv);

    // This is code for current project only, it don't need into ogon
    // ------------- Begin -------------
    if(SCARD_AUTOALLOCATE == pcbAtrLen) {
      std::lock_guard<std::mutex> lock(mtx_);
      SCARDCONTEXT hContext = Card2Context_[hCard];
      SCardFreeMemory(hContext, pAtr);
    }

    if(SCARD_AUTOALLOCATE == pcchReaderLen) {
      std::lock_guard<std::mutex> lock(mtx_);
      SCARDCONTEXT hContext = Card2Context_[hCard];
      SCardFreeMemory(hContext, szReaderName);
    }
    // -------------  End  -------------
  }

  void GetStatusChange(return_gsc& _return, const SCARDCONTEXT_RPC hContext, const DWORD_RPC dwTimeout, const std::vector<scard_readerstate_rpc> & rgReaderStates, const DWORD_RPC cReaders) {
    
    std::vector<SCARD_READERSTATE> inReaderStates(cReaders);

    for (int i = 0; i < cReaders; i++) {
      inReaderStates[i].szReader = rgReaderStates[i].szReader.c_str();
      inReaderStates[i].dwCurrentState = rgReaderStates[i].dwCurrentState;
    }

    LONG rv = SCardGetStatusChange(hContext, dwTimeout, inReaderStates.data(), cReaders);

    std::vector<scard_readerstate_rpc> outReaderStates(cReaders);

    for (int i = 0; i < cReaders; i++) {
      outReaderStates[i].dwEventState = inReaderStates[i].dwEventState;
      outReaderStates[i].rgbAtr = std::string((char*)inReaderStates[i].rgbAtr, inReaderStates[i].cbAtr);
    }

    _return.retValue = rv;
    _return.rgReaderStates = outReaderStates;
  }

  void Transmit(return_t& _return, const SCARDHANDLE_RPC hCard, const scard_io_request_rpc& pioSendPci, const LPBYTE_RPC& pbSendBuffer, const DWORD_RPC pcbRecvLength) {
    // Your implementation goes here
    printf("Transmit\n");
    SCARD_IO_REQUEST ioSendPci, ioRecvPci;

    LPCBYTE sendBuffer = (unsigned char*)pbSendBuffer.c_str();
    DWORD sendBufferLength = pbSendBuffer.length();
    DWORD recvBufferLength = pcbRecvLength;

    std::vector<char> recv;

    recv.resize(recvBufferLength);

    ioSendPci.dwProtocol = pioSendPci.dwProtocol;
    ioSendPci.cbPciLength = pioSendPci.cbPciLength;

    printf("Server received SCardTransmit: SCARDHANDLE=%ld\n", hCard);

    LONG rv = SCardTransmit(hCard, &ioSendPci, sendBuffer, sendBufferLength, &ioRecvPci, (unsigned char*)recv.data(), &recvBufferLength);

    scard_io_request_rpc ioSendPciRPC;

    ioSendPciRPC.dwProtocol = ioRecvPci.dwProtocol;
    ioSendPciRPC.cbPciLength = ioRecvPci.cbPciLength;

    _return.retValue = rv;
    _return.pioRecvPci = ioSendPciRPC;
    _return.pbRecvBuffer = std::string(recv.data(), recvBufferLength);

    printf ("SCardTransmit return %ld\n", rv);
  }

  LONG_RPC BeginTransaction(const SCARDHANDLE_RPC hCard) {
    // Your implementation goes here
    printf("BeginTransaction\n");
    return SCardBeginTransaction(hCard);
  }

  LONG_RPC EndTransaction(const SCARDHANDLE_RPC hCard, const DWORD_RPC dwDisposition) {
    // Your implementation goes here
    printf("EndTransaction\n");
    return SCardEndTransaction(hCard, dwDisposition);
  }

  void GetAttrib(return_ga& _return, const SCARDHANDLE_RPC hCard, const DWORD_RPC dwAttrId, const DWORD_RPC pcbAttrLen) {
    // Your implementation goes here

    LPBYTE pAttr = NULL;
    DWORD AttrLen = pcbAttrLen;

    std::string attrBuf;

    if(SCARD_AUTOALLOCATE != AttrLen) {

      attrBuf.resize(AttrLen);
      pAttr = (unsigned char*)attrBuf.data();
    }

    printf("Server received SCardGetAttrib: hCard=%ld, pcbAttrLen=%ld\n", hCard, pcbAttrLen);

    LONG rv = SCardGetAttrib(hCard, dwAttrId, (attrBuf.empty() ? (LPBYTE)&pAttr : pAttr), &AttrLen);

    printf("SCardGetAttrib return %ld, pAttr=%p\n", rv, pAttr);

    _return.retValue = rv;
    _return.pbAttr = std::string((char*)pAttr, AttrLen);


    // This is code for current project only, it don't need into ogon
    // ------------- Begin -------------
    if(SCARD_AUTOALLOCATE == pcbAttrLen) {
      std::lock_guard<std::mutex> lock(mtx_);
      SCARDCONTEXT hContext = Card2Context_[hCard];
      SCardFreeMemory(hContext, pAttr);
    }
    // -------------  End  -------------
  }

  void Control(return_ctrl& _return, const SCARDHANDLE_RPC hCard, const DWORD_RPC dwControlCode, const LPVOID_RPC& pbSendBuffer, const DWORD_RPC cbRecvLength) {
    
    DWORD BytesReturned;
    std::string recvBuff;

    recvBuff.resize(cbRecvLength);

    printf("Server received SCardControl: hCard=%ld, cbRecvLength=%ld\n", hCard, cbRecvLength);

    LONG rv = SCardControl(hCard, dwControlCode, pbSendBuffer.data(), pbSendBuffer.size(),  recvBuff.data(), recvBuff.size(), &BytesReturned);

    printf("SCardControl return %ld, BytesReturned=%ld\n", rv, BytesReturned);

    _return.retValue = rv;
    _return.pbRecvBuffer = std::string(recvBuff.data(), BytesReturned);
  }
  
  LONG_RPC Cancel(const SCARDCONTEXT_RPC hContext) {
    // Your implementation goes here
    printf("Cancel\n");
    return SCardCancel(hContext);
  }

  LONG_RPC IsValidContext(const SCARDCONTEXT_RPC hContext) {
    // Your implementation goes here
    printf("IsValidContext\n");
    return SCardIsValidContext(hContext);
  }
};

class OgonHandlerFactory : virtual public ogonIfFactory {
 public:
  ~OgonHandlerFactory() override = default;
  ogonIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) override
  {
    /*std::shared_ptr<TSocket> sock = std::dynamic_pointer_cast<TSocket>(connInfo.transport);
    cout << "Incoming connection\n";
    cout << "\tSocketInfo: "  << sock->getSocketInfo() << "\n";
    cout << "\tPeerHost: "    << sock->getPeerHost() << "\n";
    cout << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
    cout << "\tPeerPort: "    << sock->getPeerPort() << "\n";*/
    return new ogonHandler();
  }
  void releaseHandler( ogonIf* handler) override {
    delete handler;
  }
};

int main(int argc, char **argv) {
  int port = 9091;
  /*::std::shared_ptr<ogonHandler> handler(new ogonHandler());
  ::std::shared_ptr<TProcessor> processor(new ogonProcessor(handler));
  ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  ::std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);*/

  TThreadedServer server(
    std::make_shared<ogonProcessorFactory>(std::make_shared<OgonHandlerFactory>()),
    std::make_shared<TServerSocket>(port), //port
    std::make_shared<TBufferedTransportFactory>(),
    std::make_shared<TBinaryProtocolFactory>());

  try{
    server.serve();
  }catch(...){
    std::cout << "here";
  }
  return 0;
}

