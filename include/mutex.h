#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

typedef struct mutex {
    pthread_mutex_t themutex;
    pthread_cond_t cond_getters;
    pthread_cond_t cond_setters;
    int nb_getters;
    int nb_setters_waiting;
    int is_setting;
} mutex_t;

/**
 * @brief Initialise the mutex
*/
mutex_t Init_Mutex ();

/**
 * @brief Get an access. Note that this is a blocking function
 * @param mutex_t* mut, Pointer to the mutex
 * @param bool wSet, true if you need a get/set access, else false (only get access)
*/
void Acquire_Mutex (mutex_t* mut, bool wSet);

/**
 * @brief Release an access
 * @param mutex_t* mut, Pointer to the mutex
 * @param bool wSet, true if it was a get/set access, else false (only get access)
*/
void Release_Mutex (mutex_t* mut, bool wSet);

/**
 * @brief Destroy a mutex
 * @param mutex_t* mut, Pointer to the mutex
*/
void Free_Mutex (mutex_t* mut);

#endif  // MUTEX_H
