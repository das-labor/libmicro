#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include <libmicro/tcp_server.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif


//Convert a struct sockaddr address to a string, IPv4 and IPv6:

static char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
        switch (sa->sa_family) {
        case AF_INET:
                inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                          s, maxlen);
                break;

        case AF_INET6:
                inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                          s, maxlen);
                break;

        default:
                strncpy(s, "Unknown AF", maxlen);
                return NULL;
        }

        return s;
}

/*****************************************************************************
 * Connection management
 */

/* open a listening socket and initialize.
 * returns the socket's fd   */
static int open_listening_socket(char *port)
{
        struct addrinfo hints;
        struct addrinfo *result, *rp;
        int sfd, s;
        char buf[200];
        int ret, one = 1;

        signal(SIGPIPE, SIG_IGN);

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;  // Allow IPv4 or IPv6 - ipv4 is mapped into an v6 addr -> ai_v4mapped
        hints.ai_socktype = SOCK_STREAM;                // Datagram socket
        hints.ai_flags = AI_PASSIVE | AI_V4MAPPED | AI_NUMERICSERV;     // For wildcard IP address

        s = getaddrinfo(NULL, port, &hints, &result);
        if (s != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
        }

        /* getaddrinfo() returns a list of address structures.
           Try each address until we successfully bind(2).
           If socket(2) (or bind(2)) fails, we (close the socket
           and) try the next address. */

        for (rp = result; rp != NULL; rp = rp->ai_next) {
                sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (sfd == -1)
                        continue;

                ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
                if (ret != 0) debug_perror(0, "Could not set socket options: ");
                if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
                        break;                  /* Success */
                debug_perror(0,
                             "Could not bind to %s.",
                             get_ip_str((struct sockaddr *)rp->ai_addr,
                                        buf,
                                        sizeof(buf)));
                close(sfd);
        }

        if (rp == NULL) {               /* No address succeeded */
                debug_perror(0, "All addresses in use");
                exit(EXIT_FAILURE);
        }

        freeaddrinfo(result);           /* No longer needed */

        int flags = 0;
        flags = fcntl(sfd, F_GETFL, 0);
        fcntl(sfd, F_SETFL, flags | O_NDELAY);

        /* specify queue */
        ret = listen(sfd, SOMAXCONN);
        debug_assert(ret >= 0, "Could listen() listening socket");
        return sfd;
}


/* set bits in fd_set */
int tcp_server_fdset(tcp_server_t *serv, fd_set *set)
{
        tcp_connection_t *conn = serv->connections_head;
        int maxfd = serv->listen_socket;

        // socket for new connections
        FD_SET(serv->listen_socket, set);

        while (conn) {
                FD_SET(conn->fd, set);
                maxfd = max(maxfd, conn->fd);
                conn = conn->next;
        }

        return maxfd;
}


static void tcp_check_for_new_connections(tcp_server_t *serv, fd_set *set)
{
        tcp_connection_t *client;

        // activity on listen_socket?
        if (!FD_ISSET(serv->listen_socket, set))
                return;

        FD_CLR(serv->listen_socket, set);

        // accept connection
        int fd;
        struct sockaddr_storage remote;
        socklen_t len = sizeof(remote);

        fd = accept(serv->listen_socket, (struct sockaddr *)&remote, &len);

        // set some options on socket
        fcntl(fd, F_SETFL, O_NONBLOCK);

        debug(3, "New Client\n");
        char buf1[200];
        if (getnameinfo ((struct sockaddr *) &remote, len,
                         buf1, sizeof (buf1), NULL, 0, 0) != 0)
                strcpy (buf1, "???");
        char buf2[100];
        (void) getnameinfo ((struct sockaddr *) &remote, len,
                            buf2, sizeof (buf2), NULL, 0, NI_NUMERICHOST);
        debug(3, "connection from %s (%s)\n", buf1, buf2);

        int flag = 1;
        setsockopt(fd,
                   IPPROTO_TCP,     /* set option at TCP level */
                   TCP_NODELAY,     /* name of option */
                   (char *) &flag,  /* the cast is historical cruft */
                   sizeof(int));    /* length of option value */

        //call handler for new connections
        void *ref = serv->accept_handler(fd);

        if (ref == 0) {
                // the accept handler didn't like this connection and wants it to be closed
                debug(2, "===> Closing fd %d", fd);
                close(fd);
                return;
        }

        // initialize connection structure
        client = (tcp_connection_t *) malloc(sizeof(tcp_connection_t));
        if (client == NULL) {
                debug(0, "Could not allocate client buffer!\n");
                exit(EXIT_FAILURE);
        }

        client->fd  = fd;
        client->ref = ref;

        //insert new client at beginning of list
        client->next  = serv->connections_head;
        serv->connections_head = client;
}

void tcp_server_close_all_connections(tcp_server_t *serv)
{
        debug(1, "Closing all connections");
        tcp_connection_t *conn = serv->connections_head;
        while (conn) {
                tcp_connection_t *oldconn = conn;
                debug(1, "%s: ===> Closing fd %d", __FUNCTION__, conn->fd);

                shutdown(conn->fd, SHUT_RDWR);

                close(conn->fd);
                conn = conn->next;
                free(oldconn);
        }
}

void tcp_server_handle_activity(tcp_server_t *serv, fd_set *set)
{
        tcp_connection_t *conn = serv->connections_head;
        tcp_connection_t **link_to_conn = &serv->connections_head;

        //check for activity on connections
        while (conn) {
                if (FD_ISSET(conn->fd, set)) {
                        FD_CLR(conn->fd, set);
                        //handle it
                        int ret = serv->receive_handler(conn->fd, conn->ref);
                        if (ret != 0) { //close connection requested by handler
                                tcp_connection_t *oldconn = conn;
                                debug(1,
                                      "%s: ===> Closing fd %d",
                                      __FUNCTION__,
                                      conn->fd);

                                shutdown(conn->fd, SHUT_RDWR);

                                close(conn->fd);
                                *link_to_conn = conn->next;//remove ourselves from the list
                                conn = conn->next; //go to next element
                                free(oldconn);
                                continue;
                        }
                }
                link_to_conn = &conn->next;
                conn = conn->next;
        }

        //check for new connections
        tcp_check_for_new_connections(serv, set);
}

void tcp_server_dump_connections(tcp_server_t *serv)
{
        tcp_connection_t *client = serv->connections_head;

        while (client) {
                debug(9, "TCP connection: fd=%d", client->fd);
                client = client->next;
        }
}

tcp_server_t *new_tcp_server(
        char *port,
        tcp_receive_handler_t receive_handler,
        tcp_accept_handler_t accept_handler
)
{
        tcp_server_t *serv = (tcp_server_t *) malloc(sizeof(tcp_server_t));

        if (serv == NULL) {
                debug(0, "Could not allocate server buffer!\n");
                exit(EXIT_FAILURE);
        }

        serv->connections_head = NULL;
        serv->listen_socket = open_listening_socket(port);
        serv->receive_handler = receive_handler;
        serv->accept_handler = accept_handler;

        return serv;
}

