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
	LONG rv;

	rv = SCardEstablishContext(dwScope, NULL, NULL, &hContext);

  printf ("Server received dwScope: %ld\n", dwScope);

  g_object_set(*_return,
          "retValue", rv,
          "cardContext", hContext,
          NULL);

  return TRUE;
}

static gboolean 
my_ogon_handler_list_readers (ogonIf *iface, return_lr ** _return, const SCARDCONTEXT_RPC hContext, const LPCSTR_RPC in_mszGroups, const LPSTR_RPC in_mszReaders, const LPDWORD_RPC in_pcchReaders, GError **error){
  THRIFT_UNUSED_VAR (iface);
  THRIFT_UNUSED_VAR (error);

  /*LONG rv;

  LPCSTR mszGroups = NULL;
  LPSTR mszReaders = NULL;
  DWORD pcchReaders = SCARD_AUTOALLOCATE;

	rv = SCardListReaders(hContext, mszGroups, mszReaders, &pcchReaders);

  printf ("Server received dwScope: %ld\n", dwScope);

  g_object_set(*_return,
          "retValue", rv,
          "gchar * Readers;", hContext,
          NULL);*/

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
  ogon_handler_class->list_readers =          my_ogon_handler_list_readers;

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
