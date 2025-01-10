#ifndef IPC_H
#define IPC_H

#include "utils.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

key_t generate_key(const string&, int);

int create_shm(key_t, size_t);
void* attach_shm(int);
void detach_shm(void*);
void destroy_shm(int);

int create_sem(key_t, int);
void sem_wait(int, int);
void sem_signal(int, int);
void destroy_sem(int);

int create_msg(key_t);
void send_msg(int, long, const string&);
string receive_msg(int, long);
void destroy_msg(int);

#endif
