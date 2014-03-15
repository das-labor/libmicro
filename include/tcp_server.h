
#ifdef __MINGW32__
#define USE_WINSOCK
#endif

#ifdef USE_WINSOCK
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#endif

#include "debug.h"

// prototype for receive handler.
// fd is the fd that this connection is on, ref is the user
// supplied reference identifying the connection.
// the int that is returned shall be 0 if there is no error,
// i.e. the connection should be kept open.
// a nonzero return closes the connection. The user should clean up
// and free his ref structure in that case before returning.
typedef int (*tcp_receive_handler_t)(int fd, void *ref);


// prototype for new connection handler. this is called when the server accepts
// a new connection on the listening port.
// the function shall return a reference to the client structure the user
// identifies the individual client with. This is later passed to the receive
// handler on activity. If the funtion return 0, the connection is closed immediately.
typedef void *(*tcp_accept_handler_t)(int fd);


typedef struct tcp_connection {
        struct tcp_connection *next;
        int                   fd;

        //user reference that can be set by the new_connection handler,
        //and used by the receive handler to identify an individual client
        void                 *ref;


        //      int                               error;
} tcp_connection_t;

typedef struct {
        tcp_connection_t           *connections_head;
        int                          listen_socket;
        tcp_receive_handler_t        receive_handler;
        tcp_accept_handler_t         accept_handler;
} tcp_server_t;


/* set bits in fd_set */
int tcp_server_fdset(tcp_server_t *serv, fd_set *set);

void tcp_server_handle_activity(tcp_server_t *serv, fd_set *set);

tcp_server_t *new_tcp_server(
        char *port,
        tcp_receive_handler_t receive_handler,
        tcp_accept_handler_t accept_handler
);

void tcp_server_close_all_connections(tcp_server_t *serv);

