#include "setup.h"

int main () {
    pid_t child = fork ();

    if (child == 0) {

        ptrace (PTRACE_TRACEME, 0, NULL, NULL);
        execl ("a.out", "a", NULL);

    } else {

        webserver_t* webserver = Init_Webserver (80);
        mem_array_t child_mem = Init_Mem_Array ();
        mutex_t child_mem_mut = Init_Mutex ();

        // Webserver
        pthread_t ws_thread;
        pthread_create (&ws_thread, NULL, Listening_Webserver, (void*) webserver);

        // Display Mem
        pthread_t display_mem_thread;
        struct display_mem_args d_mem_args = {child, webserver, &child_mem, &child_mem_mut};
        pthread_create (&display_mem_thread, NULL, Display_Mem, (void*) &d_mem_args);

        // Trace the program
        Trace (child, &child_mem, &child_mem_mut);

        // Wait for everything
        pthread_join (ws_thread, NULL);
        pthread_join (display_mem_thread, NULL);

        // Free everything
        Free_Mem_Array (&child_mem);
        Free_Mutex (&child_mem_mut);
        Free_Webserver (webserver);

    }

    return 0;
}