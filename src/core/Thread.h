#ifndef THREAD_H
#define THREAD_H

#include <zephyr/kernel.h>
#include <stdlib.h>
#include "ThreadCommand.h"

#define MAX_QUEUE_DEPTH 10

class Thread {
protected:
    k_tid_t thread_id;
    struct k_msgq message_queue;
    ThreadMessage message_queue_buffer[MAX_QUEUE_DEPTH];
    Thread(void);

private:
    // need to delete copy cstor and assign optor
    Thread(const Thread &) = delete;
    Thread& operator=(const Thread&) = delete;

public:
    bool SendMessage(ThreadMessage msg);
    virtual void Initialize() = 0;
    static void RunThreadSequence(void* instance, void*, void*);
    virtual void Run() = 0;

};

#endif