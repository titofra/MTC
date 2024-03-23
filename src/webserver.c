#include "webserver.h"

webserver_t Init_Webserver (const uint16_t port) {
    webserver_t ws;
    
    // Create socket
    ws.sck = socket (AF_INET, SOCK_STREAM, 0);
    if (ws.sck < 0) {
        perror ("[Webserver] socket");
        exit (1);
    }

    // Forcefully attaching the socket to the port
    int opt = 1;
    if (setsockopt (
        ws.sck,
        SOL_SOCKET,
        SO_REUSEADDR | SO_REUSEPORT,
        &opt,
        sizeof (opt))
    ){
        perror ("[Webserver] setsockopt");
        exit (1);
    }

    // Set port/ip
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons (port);
    addr.sin_addr.s_addr = INADDR_ANY;

    // Bind to port/ip
    if (bind (ws.sck, (struct sockaddr*) &addr, sizeof (addr)) < 0){
        perror ("[Webserver] bind");
        exit (1);
    }

    return ws;
}

void* Listening_Webserver (void* args) {
    webserver_t* ws = (webserver_t*) args;

    if (listen (ws->sck, MAX_SCK_REQ_QUEUED) < 0) {
        perror ("[Webserver] listen");
        exit (EXIT_FAILURE);
    }

    printf ("[Webserver] listening...\n");

    struct sockaddr_in cli_addr;
    socklen_t cli_size = sizeof (cli_addr);
    char buffer [BUFFER_SZ];
    int connfd;
    while (1) {
        // Accept an incoming connection
        connfd = accept (ws->sck, (struct sockaddr*) &cli_addr, &cli_size);
        if (connfd > 0) {
            // Read incoming request
            if (read (connfd, buffer, BUFFER_SZ) > 0) {
                printf ("[Webserver] New connection from %s:%i\n",
                    inet_ntoa (cli_addr.sin_addr),
                    ntohs (cli_addr.sin_port)
                );
                _Handle_Client_Webserver (connfd, buffer);
            } else {
                printf ("[Webserver] Failed to receive packet from %s:%i\n",
                    inet_ntoa (cli_addr.sin_addr),
                    ntohs (cli_addr.sin_port)
                );
            }
        } else {
            printf ("[Webserver] Cannot accept the client %s:%i\n",
                inet_ntoa (cli_addr.sin_addr),
                ntohs (cli_addr.sin_port)
            );
        }
        close (connfd);
    }

    return NULL;
}

void _Handle_Client_Webserver (int connfd, char* buffer) {
    // Parse requested URL path and extract query string arguments
    char method [8], path [256], protocol [16];
    sscanf (buffer, "%s %s %s", method, path, protocol);

    if (strcmp (method, "GET") != 0) {
        send (connfd, "HTTP/1.1 405 Method Not Allowed\r\n\r\n", strlen ("HTTP/1.1 405 Method Not Allowed\r\n\r\n"), 0);
        return;
    }

    char html [2048];
    sprintf (html, "<html><body>aaaaaa</body></html>\r\n");

    // Send the HTML back to the client
    send (connfd, html, strlen (html), 0);
}
