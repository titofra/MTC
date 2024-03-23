#include "mutex.h"

void InitMutex (mutex_t* mut) {
    if (pthread_mutex_init(&mut->themutex, NULL) != 0) {
        perror ("Mutex init failed\n");
        exit (1);
    }
    if (pthread_cond_init(&mut->cond_getters, NULL) != 0)
    {
        perror ("Mutex Cond init failed\n");
        exit (1);
    }
    if (pthread_cond_init(&mut->cond_setters, NULL) != 0)
    {
        perror ("Mutex Cond init failed\n");
        exit (1);
    }
    mut->nb_getters = 0;
    mut->nb_setters_waiting = 0;
    mut->is_setting = 0;
}

void AcquireMutex (mutex_t* mut, bool wSet) {
    pthread_mutex_lock(&mut->themutex);
    if (wSet) {
        mut->nb_setters_waiting ++;
        // If there are any getters holding the mutex or if a setter is currently holding the mutex,
        // wait on the condition variable for setters until the mutex is released.
        while (mut->nb_getters > 0 || mut->is_setting) {
            pthread_cond_wait(&mut->cond_setters, &mut->themutex);
        }
        mut->nb_setters_waiting --;
        mut->is_setting = 1;
    } else {
        // If there are any setters waiting or if a setter is currently holding the mutex,
        // wait on the condition variable for getters until a setter releases the mutex.
        while (mut->nb_setters_waiting > 0 || mut->is_setting) {
            pthread_cond_wait(&mut->cond_getters, &mut->themutex);
        }
        mut->nb_getters++;
    }
    pthread_mutex_unlock(&mut->themutex);
}

void ReleaseMutex (mutex_t* mut, bool wSet) {
    pthread_mutex_lock(&mut->themutex);
    if (wSet) {
        mut->is_setting = 0;
        // Broadcast to the condition variable for getters to wake up any waiting getters.
        pthread_cond_broadcast(&mut->cond_getters);
    } else {
        mut->nb_getters--;
        // If there are no more getters holding the mutex, signal the condition variable for setters
        // to wake up any waiting setters.
        if (mut->nb_getters == 0) {
            pthread_cond_signal(&mut->cond_setters);
        }
    }
    pthread_mutex_unlock(&mut->themutex);
}

void DestroyMutex(mutex_t* mut) {
    pthread_mutex_destroy(&mut->themutex);
    pthread_cond_destroy(&mut->cond_getters);
    pthread_cond_destroy(&mut->cond_setters);
}