#include "gen-cpp/ogon.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

//#include <PCSC/winscard.h>
#include <PCSC/winscard.h>

class ogonHandler : virtual public ogonIf {
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

  void ListReaders(return_lr& _return, const SCARDCONTEXT_RPC hContext) {
    // Your implementation goes here
    printf("ListReaders\n");

    LPSTR mszReaders = NULL;
    DWORD pcchReaders = SCARD_AUTOALLOCATE;

    printf ("Server received SCardListReaders: SCARDCONTEXT=%ld\n", hContext);

    LONG rv = SCardListReaders(hContext, NULL, (LPSTR)&mszReaders, &pcchReaders);

    printf ("SCardListReaders return %ld, Server send list readers=%s\n", rv, mszReaders);

    _return.retValue = rv;
    _return.mszReaders = mszReaders;
    _return.pcchReaders = pcchReaders;

    SCardFreeMemory(hContext, mszReaders);
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
    
    return rv;
  }

  void Status(return_s& _return, const SCARDHANDLE_RPC hCard) {
    // Your implementation goes here

    char Reader[MAX_READERNAME];
    DWORD ReaderLen = MAX_READERNAME;
    DWORD pdwState;
    DWORD pdwProtocol;
    BYTE pbAtr[MAX_ATR_SIZE] = "";
    DWORD pcbAtrLen = MAX_ATR_SIZE;

    printf("Server received SCardStatus: SCARDHANDLE=%ld\n", hCard);

    LONG rv = SCardStatus(hCard, Reader, &ReaderLen, &pdwState, &pdwProtocol, pbAtr, &pcbAtrLen);

    _return.retValue = rv;
    _return.szReaderName = Reader;
    _return.pcchReaderLen = ReaderLen;
    _return.pdwState = pdwState;
    _return.pdwProtocol = pdwProtocol;
    _return.pbAtr = std::string((char*)pbAtr, pcbAtrLen);

    printf ("SCardStatus return %ld\n", rv);
  }

  void GetStatusChange(return_gsc& _return, const SCARDHANDLE_RPC hCard, const DWORD_RPC dwTimeout, const std::vector<scard_readerstate_rpc> & rgReaderStates, const DWORD_RPC cReaders) {
    
    // Your implementation goes here
    //LONG rv = SCardGetStatusChange(hCard, Reader, &ReaderLen, &pdwState, &pdwProtocol, pbAtr, &pcbAtrLen);

    //LONG rv = SCardGetStatusChange(SCARDCONTEXT hContext, DWORD dwTimeout,
		//SCARD_READERSTATE *rgReaderStates, DWORD cReaders)
  
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

    //GByteArray *recv = g_byte_array_new();
    //recv = g_byte_array_set_size(recv, pcbRecvLength);

    LONG rv = SCardTransmit(hCard, &ioSendPci, sendBuffer, sendBufferLength, &ioRecvPci, (unsigned char*)recv.data(), &recvBufferLength);

    //recv = g_byte_array_set_size(recv, recvBufferLength);
    scard_io_request_rpc ioSendPciRPC;

    ioSendPciRPC.dwProtocol = ioRecvPci.dwProtocol;
    ioSendPciRPC.cbPciLength = ioRecvPci.cbPciLength;

    _return.retValue = rv;
    _return.pioRecvPci = ioSendPciRPC;
    _return.pbRecvBuffer = std::string(recv.data(), recvBufferLength);

    printf ("SCardTransmit return %ld\n", rv);
  }

};

int main(int argc, char **argv) {
  int port = 9091;
  ::std::shared_ptr<ogonHandler> handler(new ogonHandler());
  ::std::shared_ptr<TProcessor> processor(new ogonProcessor(handler));
  ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  ::std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

