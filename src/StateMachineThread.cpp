#include <StateMachineThread.h>
#include <state_machine.h>

LOG_MODULE_REGISTER(StateMachineThread, LOG_LEVEL_INF);
K_THREAD_STACK_DEFINE(state_machine_thread_area, STATE_MACHINE_THREAD_STACK_SIZE);

struct k_thread state_machine_thread_data;


StateMachineThread::StateMachineThread() {};


void StateMachineThread::Initialize() {
    if (id == nullptr) {
        LOG_DBG("StateMachine::%s -- making thread", __FUNCTION__);
        id = k_thread_create(
            &state_machine_thread_data, state_machine_thread_area,
            K_THREAD_STACK_SIZEOF(state_machine_thread_area),
            StateMachineThread::RunThreadSequence,
            this, NULL, NULL,
            STATE_MACHINE_THREAD_PRIORITY, 0, K_NO_WAIT
        );

        k_thread_name_set(id, "StateMachineThread");
        LOG_DBG("StateMachine::%s -- thread create successful", __FUNCTION__);
    }
}


void StateMachineThread::Run() {
    // Initialize State Machine
    state_machine_init();
}