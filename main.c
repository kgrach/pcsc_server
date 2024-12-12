#include "gen-c_glib/ogon.h"

#include <thrift/c_glib/server/thrift_simple_server.h>
#include <thrift/c_glib/transport/thrift_server_socket.h>
#include <thrift/c_glib/transport/thrift_buffered_transport_factory.h>
#include <thrift/c_glib/protocol/thrift_binary_protocol_factory.h>
#include <thrift/c_glib/protocol/thrift_protocol_factory.h>
#include <thrift/c_glib/transport/thrift_server_transport.h>
#include <thrift/c_glib/thrift.h>

#include <glib-object.h>
#include <glib.h>

#include <stdio.h>

#include <PCSC/winscard.h>
#include <PCSC/pcsclite.h>

G_BEGIN_DECLS

#define TYPE_MY_OGON_HANDLER                          \
  (my_ogon_handler_get_type ())
#define MY_OGON_HANDLER(obj)                          \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                           \
                               TYPE_MY_OGON_HANDLER,  \
                               MyOgonHandler))
#define MY_OGON_HANDLER_CLASS(c)                      \
  (G_TYPE_CHECK_CLASS_CAST ((c),                                \
                            TYPE_MY_OGON_HANDLER,     \
                            MyOgonHandlerClass))
#define IS_MY_OGON_HANDLER(obj)                       \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                           \
                               TYPE_MY_OGON_HANDLER))
#define IS_MY_OGON_HANDLER_CLASS(c)                   \
  (G_TYPE_CHECK_CLASS_TYPE ((c),                                \
                            TYPE_MY_OGON_HANDLER))
#define MY_OGON_HANDLER_GET_CLASS(obj)                \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                            \
                              TYPE_MY_OGON_HANDLER,   \
                              MyOgonHandlerClass))

struct _MyOgonHandler {
  ogonHandler parent_instance;
};
typedef struct _MyOgonHandler MyOgonHandler;

struct _MyOgonHandlerClass {
  ogonHandlerClass parent_class;
};
typedef struct _MyOgonHandlerClass MyOgonHandlerClass;

GType my_ogon_handler_get_type (void);

G_END_DECLS

G_DEFINE_TYPE (MyOgonHandler,
               my_ogon_handler,
               TYPE_OGON_HANDLER)

static gboolean 
my_ogon_handler_establish_context (ogonIf *iface, return_ec ** _return, const DWORD_RPC dwScope, GError **error) {
  THRIFT_UNUSED_VAR (iface);
  THRIFT_UNUSED_VAR (error);

	SCARDCONTEXT hContext;

  printf ("Server received SCardEstablishContext dwScope: %ld\n", dwScope);

	LONG rv = SCardEstablishContext(dwScope, NULL, NULL, &hContext);

  printf ("SCardEstablishContext return %ld, Server send hContext: %ld\n", rv, hContext);

  g_object_set(*_return,
          "retValue", rv,
          "cardContext", hContext,
          NULL);

  return TRUE;
}

static gboolean 
my_ogon_handler_release_context(ogonIf *iface, LONG_RPC* _return, const SCARDCONTEXT_RPC hContext, GError **error ){

  THRIFT_UNUSED_VAR (iface);
  THRIFT_UNUSED_VAR (error);

  printf ("Server received SCardReleaseContext dwScope: %ld\n", hContext);

	LONG rv = SCardReleaseContext(hContext);

  printf ("SCardReleaseContext return %ld\n", rv);

  *_return = rv;

  return TRUE;
}

static gboolean 
my_ogon_handler_list_readers (ogonIf *iface, return_lr ** _return, const SCARDCONTEXT_RPC hContext, GError **error){
  THRIFT_UNUSED_VAR (iface);
  THRIFT_UNUSED_VAR (error);

  LPSTR mszReaders = NULL;
  DWORD pcchReaders = SCARD_AUTOALLOCATE;

  printf ("Server received SCardListReaders: SCARDCONTEXT=%ld\n", hContext);

	LONG rv = SCardListReaders(hContext, NULL, (LPSTR)&mszReaders, &pcchReaders);

  printf ("SCardListReaders return %ld, Server send list readers=%s\n", rv, mszReaders);

  g_object_set(*_return,
          "retValue", rv,
          "mszReaders", mszReaders,
          "pcchReaders", pcchReaders,
          NULL);

  SCardFreeMemory(hContext, mszReaders);
  
  return TRUE;
}

static gboolean 
my_ogon_handler_connect (ogonIf *iface, return_c ** _return, const SCARDCONTEXT_RPC hContext, const LPCSTR_RPC szReader, const DWORD_RPC dwShareMode, const DWORD_RPC dwPreferredProtocols, GError **error) {
  THRIFT_UNUSED_VAR (iface);
  THRIFT_UNUSED_VAR (error);

  SCARDHANDLE phCard;
  DWORD pdwActiveProtocol;

  printf ("Server received SCardConnect: SCARDCONTEXT=%ld\n", hContext);

	LONG rv = SCardConnect(hContext, szReader, dwShareMode, dwPreferredProtocols, &phCard, &pdwActiveProtocol);

  printf ("SCardConnect return %ld, Server send SCARDHANDLE=%ld\n", rv, phCard);

  g_object_set(*_return,
          "retValue", rv,
          "phCard", phCard,
          "pdwActiveProtocol", pdwActiveProtocol,
          NULL);
  
  return TRUE;
}

static gboolean 
my_ogon_handler_reconnect (ogonIf *iface, return_r ** _return, const SCARDHANDLE_RPC hCard, const DWORD_RPC dwShareMode, const DWORD_RPC dwPreferredProtocols, const DWORD_RPC dwInitialization, GError **error) {
  THRIFT_UNUSED_VAR (iface);
  THRIFT_UNUSED_VAR (error);

  DWORD pdwActiveProtocol;

  printf ("Server received SCardReconnect: SCARDHANDLE=%ld\n", hCard);

	LONG rv = SCardReconnect(hCard, dwShareMode, dwPreferredProtocols, dwInitialization, &pdwActiveProtocol);

  printf ("SCardReconnect return %ld, Server send pdwActiveProtocol=%ld\n", rv, pdwActiveProtocol);

  g_object_set(*_return,
          "retValue", rv,
          "pdwActiveProtocol", pdwActiveProtocol,
          NULL);
  
  return TRUE;
}

static gboolean
my_ogon_handler_disconnect (ogonIf *iface, LONG_RPC* _return, const SCARDHANDLE_RPC hCard, const DWORD_RPC dwDisposition, GError **error) {
  THRIFT_UNUSED_VAR (iface);
  THRIFT_UNUSED_VAR (error);

  printf ("Server received SCardDisconnect: SCARDHANDLE=%ld\n", hCard);

  LONG rv = SCardDisconnect(hCard, dwDisposition);

  printf ("SCardDisconnect return %ld\n", rv);
  
  *_return = rv;

  return TRUE;
}

static gboolean 
my_ogon_handler_status (ogonIf *iface, return_s ** _return, const SCARDHANDLE_RPC hCard, GError **error) {
  THRIFT_UNUSED_VAR (iface);
  THRIFT_UNUSED_VAR (error);

  char Reader[MAX_READERNAME];
  DWORD ReaderLen = MAX_READERNAME;
  DWORD pdwState;
  DWORD pdwProtocol;
  BYTE pbAtr[MAX_ATR_SIZE] = "";
  DWORD pcbAtrLen = MAX_ATR_SIZE;

  printf("Server received SCardStatus: SCARDHANDLE=%ld\n", hCard);

  LONG rv = SCardStatus(hCard, Reader, &ReaderLen, &pdwState, &pdwProtocol, pbAtr, &pcbAtrLen);

  GByteArray *atr = g_byte_array_new();
  atr = g_byte_array_append(atr, pbAtr, pcbAtrLen);

  g_object_set(*_return,
        "retValue", rv,
        "szReaderName", Reader,
        "pcchReaderLen", ReaderLen,
        "pdwState", pdwState, 
        "pdwProtocol", pdwProtocol,
        "pbAtr", atr,
        NULL);

  printf ("SCardStatus return %ld\n", rv);
  
  return TRUE;
}

static gboolean 
my_ogon_handler_transmit (ogonIf *iface, return_t ** _return, const SCARDHANDLE_RPC hCard, const scard_io_request_rpc * pioSendPci, const LPBYTE_RPC pbSendBuffer, const DWORD_RPC pcbRecvLength, GError **error) {
  THRIFT_UNUSED_VAR (iface);
  THRIFT_UNUSED_VAR (error);

  SCARD_IO_REQUEST ioSendPci, ioRecvPci;

  LPCBYTE sendBuffer = pbSendBuffer->data;
  DWORD sendBufferLength = pbSendBuffer->len;
  DWORD recvBufferLength;

  ioSendPci.dwProtocol = pioSendPci->dwProtocol;
  ioSendPci.cbPciLength = pioSendPci->cbPciLength;

  printf("Server received SCardTransmit: SCARDHANDLE=%ld\n", hCard);

  GByteArray *recv = g_byte_array_new();
  recv = g_byte_array_set_size(recv, pcbRecvLength);

  LONG rv = SCardTransmit(hCard, &ioSendPci, sendBuffer, sendBufferLength, &ioRecvPci, recv->data, &recvBufferLength);

  recv = g_byte_array_set_size(recv, recvBufferLength);

  scard_io_request_rpc *ioSendPciRPC = g_object_new(TYPE_SCARD_IO_REQUEST_RPC, NULL);

  ioSendPciRPC->dwProtocol = ioRecvPci.dwProtocol;
  ioSendPciRPC->cbPciLength = ioRecvPci.cbPciLength;

  g_object_set(*_return,
        "retValue", rv,
        "pbRecvBuffer",    recv,
        "pioRecvPci", ioSendPciRPC,
        NULL);

  printf ("SCardTransmit return %ld\n", rv);
  
  return TRUE;
}

static void
my_ogon_handler_init (MyOgonHandler *self)
{
}

static void
my_ogon_handler_class_init (MyOgonHandlerClass *klass)
{
  ogonHandlerClass *ogon_handler_class =
    OGON_HANDLER_CLASS (klass);

  ogon_handler_class->establish_context =     my_ogon_handler_establish_context;
  ogon_handler_class->release_context =       my_ogon_handler_release_context;
  ogon_handler_class->list_readers =          my_ogon_handler_list_readers;
  ogon_handler_class->connect =               my_ogon_handler_connect;
  ogon_handler_class->reconnect =             my_ogon_handler_reconnect;
  ogon_handler_class->disconnect =            my_ogon_handler_disconnect;
  ogon_handler_class->status =                my_ogon_handler_status;
  ogon_handler_class->transmit =              my_ogon_handler_transmit;
}

int
main (void)
{
  ogonHandler *handler;
  ogonProcessor *processor;

  ThriftServerTransport *server_transport;
  ThriftTransportFactory *transport_factory;
  ThriftProtocolFactory *protocol_factory;
  ThriftServer *server;

  GError *error = NULL;

#if (!GLIB_CHECK_VERSION (2, 36, 0))
  g_type_init ();
#endif

  handler   = g_object_new (TYPE_MY_OGON_HANDLER,
                            NULL);
  processor = g_object_new (TYPE_OGON_PROCESSOR,
                            "handler", handler,
                            NULL);

  server_transport  = g_object_new (THRIFT_TYPE_SERVER_SOCKET,
                                    "port", 9091,
                                    NULL);
  transport_factory = g_object_new (THRIFT_TYPE_BUFFERED_TRANSPORT_FACTORY,
                                    NULL);
  protocol_factory  = g_object_new (THRIFT_TYPE_BINARY_PROTOCOL_FACTORY,
                                    NULL);

  server = g_object_new (THRIFT_TYPE_SIMPLE_SERVER,
                         "processor",                processor,
                         "server_transport",         server_transport,
                         "input_transport_factory",  transport_factory,
                         "output_transport_factory", transport_factory,
                         "input_protocol_factory",   protocol_factory,
                         "output_protocol_factory",  protocol_factory,
                         NULL);

  thrift_server_serve (server, &error);

  fprintf (stderr, "Client caught an exception: %s\n", error->message);
  g_clear_error (&error);

  g_object_unref (server);
  g_object_unref (protocol_factory);
  g_object_unref (transport_factory);
  g_object_unref (server_transport);

  g_object_unref (processor);
  g_object_unref (handler);

  return 0;
}
