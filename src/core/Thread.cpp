#include "Thread.h"

Thread::Thread() {
    // TODO
    if (k_msgq_init(&message_queue, message_queue_buffer, MAX_QUEUE_DEPTH, MESSAGE_SIZE_BYTES) != 0) {
        // TODO: debug message err
        return;
    }
}

bool Thread::SendMessage(ThreadMessage msg) {
    if (k_msgq_put(&message_queue, &msg, K_NO_WAIT) != 0) {
        // TODO: uart debug message send failed
        return false;
    }
    return true;
}

// This is a static function required by k_thread_create
void Thread::RunThreadSequence(void* instance, void*, void*) {
    // HAVE to receive an instance ptr to run the correct thread sequence
    // because static functions are not associated with thread objects 
    static_cast<Thread*>(instance)->Run();
}