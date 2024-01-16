#pragma once

#include <zephyr/kernel.h>

template <typename element_type, size_t depth> class MessageQueue {
private:
    size_t msg_size_B;
    size_t queue_depth_B;
    char message_buffer[depth * sizeof(element_type)]; 
    struct k_msgq queue;
public:
    MessageQueue() {
        msg_size_B = sizeof(element_type);
        queue_depth_B = msg_size_B * depth;
        k_msgq_init(&queue, message_buffer, depth, msg_size_B);
    };
    ~MessageQueue() = default;

    bool push(const element_type& msg) {
        if (k_msgq_put(&queue, static_cast<void*>(&msg), K_NO_WAIT) != 0) {
            // TODO: uart debug message send failed
            return false;
        }
        return true;
    };

    bool push_with_timeout(const element_type& msg, int64_t ticks) {
        if (k_msgq_put(&queue, static_cast<void*>(&msg), k_timeout_t(ticks)) != 0) {
            return false;
        }
        return true;
    };

    bool get(element_type& msg) {
        // NOTE: this MUST be used if accessing queue from ISR
        if (k_msgq_get(&queue, static_cast<void*>(&msg), K_NO_WAIT) != 0) {
            return false;
        }
        return true;
    }

    bool get_with_timeout(element_type& msg, int64_t ticks) {
        if (k_msgq_get(&queue, static_cast<void*>(&msg), k_timeout_t(ticks)) != 0) {
            return false;
        }
        return true;
    }

    bool peak(element_type& msg) {
        if (k_msgq_peek(&queue, static_cast<void*>(&msg)) != 0) {
            return false;
        }
        return true;
    };

    bool peak_at(element_type& msg, uint32_t idx) {
        if (k_msgq_peek_at(&queue, static_cast<void*>(&msg), idx) != 0) {
            return false;
        }
        return true;
    };  

    void resetMessageQueue() {
        k_msgq_purge(&queue);
    };
};