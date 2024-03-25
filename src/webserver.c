#include "webserver.h"

unsigned char http_response [2048] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n\
<!DOCTYPE html>\
<html>\
    <body>\
    </body>\
    <script>\
        const socket = new WebSocket (window.location.origin.replace(/^http/, 'ws'), 'MTC_prot');\
        socket.addEventListener ('open', (event) => {\
            console.log ('Connection established:', event);\
        });\
        socket.addEventListener ('message', (event) => {\
            console.log('Message received:', event.data);\
        });\
        socket.addEventListener ('close', (event) => {\
            console.log('Connection closed:', event);\
        });\
        socket.addEventListener ('error', (error) => {\
            console.error('Error occurred:', error);\
        });\
        function sendData (data) {\
            if (!socket || socket.readyState !== WebSocket.OPEN) return;\
            socket.send(data);\
            console.log(`Sent: `, data);\
        }\
    </script>\
</html>\0";

// TODO: libwesockets's user pointer seems not working 
char* data_shared = NULL;
size_t len_data_shared = 0;
mutex_t mut_data_shared;

static int callback_http_Webserver (struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_HTTP:
            if (len <= 0 || ((char *)in) [len] != '\0') return 1;
            if (strlen ((char *) in) <= 1) {
                if (lws_write (wsi, http_response, strlen ((char*) http_response), LWS_WRITE_HTTP_FINAL) < 0) {
                    lwsl_err ("[Webserver] lws_write");
                }
            }
            return 1;
            break;
        default:
            break;
    }

    return 0;   // close the connection

    UNUSED (user);
    UNUSED (in);
    UNUSED (len);
}

static int callback_websocket_Webserver (struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf ("[Webserver] WebSocket connection established.\n");
            break;
        case LWS_CALLBACK_CLOSED:
            printf ("[Webserver] WebSocket connection closed.\n");
            break;
        case LWS_CALLBACK_RECEIVE:
            ;
            char* msg = (char*) malloc ((len + 1) * sizeof (char));
            strncpy (msg, (char*) in, len);
            msg [len] = '\0';
            printf ("[Webserver] Received message: %s\n", msg);
            free (msg);
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
            Acquire_Mutex (&mut_data_shared, false);
            if (len_data_shared > 0) {
                char buf [LWS_PRE + len_data_shared];
                memcpy (&buf [LWS_PRE], data_shared, len_data_shared);
                Release_Mutex (&mut_data_shared, false);

                if (lws_write (wsi, (unsigned char*) &buf [LWS_PRE], len_data_shared, LWS_WRITE_BINARY) < 0) {
                    lwsl_err ("[Webserver] lws_write");
                }
            } else {
                Release_Mutex (&mut_data_shared, false);
            }
            break;
        default:
            break;
    }

    return 0;

    UNUSED (user);
}

webserver_t* Init_Webserver (int port) {
    webserver_t* ws = (webserver_t*) malloc (sizeof (webserver_t));

    mut_data_shared = Init_Mutex ();

    ws->protocols [0] = (struct lws_protocols) {
        .name = "http-only",
        .callback = &callback_http_Webserver,
        .per_session_data_size = 0,
        .rx_buffer_size = 0,
    };
    ws->protocols [1] = (struct lws_protocols) {
        .name = "MTC_prot",
        .callback = &callback_websocket_Webserver,
        .per_session_data_size = 0,
        .rx_buffer_size = 0,
    };
    ws->protocols [2] = (struct lws_protocols) {NULL, NULL, 0, 0, 0, NULL, 0};

    signal (SIGPIPE, SIG_IGN);

    struct lws_context_creation_info info;
    memset (&info, 0, sizeof (info));
    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT | LWS_SERVER_OPTION_VALIDATE_UTF8;
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.iface = NULL;  // all the interfaces
    info.protocols = ws->protocols;
    info.extensions = NULL;
    info.port = port;

    ws->context = lws_create_context (&info);

    if (!ws->context) {
        perror ("[Webserver] lws_create_context");
        exit (1);
    }

    return ws;
}

void* Listening_Webserver (void* args) {
    webserver_t *ws = (webserver_t*) args;

    printf ("[Webserver] Listening ...\n");
    while (1) {
        lws_service (ws->context, 50);
    }

    return NULL;
}

void Send_Webserver (webserver_t* ws, const char* data, size_t len) {
    Acquire_Mutex (&mut_data_shared, true);
        len_data_shared = len;
        data_shared = realloc (data_shared, len_data_shared * sizeof (char));
        strncpy (data_shared, data, len_data_shared);
    Release_Mutex (&mut_data_shared, true);

    lws_callback_on_writable_all_protocol (ws->context, &ws->protocols [1]);
}

void Free_Webserver (webserver_t* ws) {
    lws_context_destroy (ws->context);
    Free_Mutex (&mut_data_shared);
    free (data_shared);
    free (ws);
}
