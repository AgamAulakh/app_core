#pragma once

#include <zephyr/kernel.h>
#include <stdlib.h>
#include "MessageQueue.h"

template <size_t message_queue_depth> class Thread {
protected:
    k_tid_t id;
    MessageQueue<uint8_t, message_queue_depth> message_queue;    

public:
    Thread() : id(nullptr) {}; 
    virtual ~Thread() {};

    virtual void Initialize() = 0;
    virtual void Run () = 0;

    bool SendMessage(uint8_t msg) {
        if (message_queue.push(msg) == false) {
            // TODO: uart debug message send failed
            return false;
        }
        return true;
    };

    // This is a static function required by k_thread_create
    // NOTE: can't have a virtual static function, need this to be defined for all
    static void RunThreadSequence(void* instance, void*, void*) {
        // HAVE to receive an instance ptr to run the correct thread sequence
        // because static functions are not associated with thread objects 
        static_cast<Thread*>(instance)->Run();
    };

    //// NOTE: MAY need to add killing/re-starting threads
    // void Start();
    // void Kill();
};