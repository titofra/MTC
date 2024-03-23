#include "setup.h"

int main () {
    pid_t child = fork ();

    if (child == 0) {

        ptrace (PTRACE_TRACEME, 0, NULL, NULL);
        execl ("a.out", "a", NULL);

    } else {

        mem_array_t child_mem = Init_Mem_Array ();
        webserver_t webserver = Init_Webserver (80);

        // Webserver
        pthread_t ws_thread;
        pthread_create (&ws_thread, NULL, Listening_Webserver, (void*) &webserver);

        // Trace the program
        Trace (child, &child_mem);

        // Wait for everything
        pthread_join (ws_thread, NULL);

        // Free everything
        Free_Mem_Array (&child_mem);

    }

    return 0;
}