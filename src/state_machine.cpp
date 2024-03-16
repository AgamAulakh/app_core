#include <zephyr/smf.h>
#include <state_machine.h>
#include <led_handler.h>

LOG_MODULE_REGISTER(state_machine, LOG_LEVEL_DBG);

/* Forward declaration of state table */
extern const struct smf_state dev_states[];

enum dev_state { INIT, IDLE, TEST, PROCESS, COMPLETE, CANCEL };

/* User defined object */
struct s_object {
    struct smf_ctx ctx;

    /* Events */
    struct k_event button_press_event;
    int32_t events;

} s_obj;

void StateMachine::init_run(void *obj) {
    // Setup threads
    LOG_DBG("init run state");

    smf_set_state(SMF_CTX(&s_obj), &dev_states[IDLE]);
};

void StateMachine::idle_entry(void *obj) {
    LOG_DBG("idle entry state");
    LED1::set_blue();

    // Clear any remaining button press if not yet cleared
    if (s_obj.events & EVENT_BTN_PRESS) {
        smf_set_state(SMF_CTX(&s_obj), &dev_states[TEST]);
        s_obj.events = k_event_clear(&s_obj.button_press_event, EVENT_BTN_PRESS);
    }
};

void StateMachine::idle_run(void *obj) {
    LOG_DBG("idle run state");

    /* If the button is pressed the user wants to start a test,
    move to TEST state */
    s_obj.events = k_event_wait(&s_obj.button_press_event, EVENT_BTN_PRESS, true, K_FOREVER);

    // Clear button press
    if (s_obj.events & EVENT_BTN_PRESS) {
        smf_set_state(SMF_CTX(&s_obj), &dev_states[TEST]);
        s_obj.events = k_event_clear(&s_obj.button_press_event, EVENT_BTN_PRESS);
    }
};

void StateMachine::test_entry(void *obj) {
    LOG_DBG("test entry state");
    LED1::set_yellow();
};

void StateMachine::test_run(void *obj) {
    // Start test - collect data
    // begin collecting data from AFE
    // begin data processing
    LOG_DBG("test run state");

    // Have this here a second time as for some reason it does not clear
    // properly when in the previous state?
    s_obj.events = k_event_clear(&s_obj.button_press_event, EVENT_BTN_PRESS);

    /* We need to wait for either a button press to cancel or callback from sigproc
    data collection so we know which state to go to */
    // s_obj.events = k_event_wait(&s_obj.button_press_event, EVENT_BTN_PRESS, true, K_FOREVER);

    /* If the button is pressed the user wants to terminate
    the test, move to CANCEL state */
    if (s_obj.events & EVENT_BTN_PRESS) {
        smf_set_state(SMF_CTX(&s_obj), &dev_states[CANCEL]);
        s_obj.events = k_event_clear(&s_obj.button_press_event, EVENT_BTN_PRESS);
    }
    else {
        // Once data collection is done move to PROCESS state
        smf_set_state(SMF_CTX(&s_obj), &dev_states[PROCESS]);
    }
};

void StateMachine::test_exit(void *obj) {
    // Stop test - stop data collection
    LOG_DBG("test exit state");

    /* If the user wants to terminate the test, stop
    data processing and discard data */
    if (s_obj.ctx.current == &dev_states[CANCEL]) {
        // stop data processing
    }
};

void StateMachine::process_entry(void *obj) {
    LOG_DBG("process entry state");
};

void StateMachine::process_run(void *obj) {
    // Run signal processing
    LOG_DBG("process run state");

    // Remove once sigproc can signal to state machine to change states
    static uint8_t tempCounter = 5;

    LED1::set_flash_green();

    /* We need to wait for either a button press to cancel or callback from sigproc
    data collection so we know which state to go to */
    // s_obj.events = k_event_wait(&s_obj.button_press_event, EVENT_BTN_PRESS, true, K_FOREVER);
    LOG_DBG("button press %ld",(s_obj.events & EVENT_BTN_PRESS));
    if (s_obj.events & EVENT_BTN_PRESS) {
        /* If the button is pressed the user wants to terminate
        the test, move to CANCEL state */
        LOG_DBG("cancel processing");
        smf_set_state(SMF_CTX(&s_obj), &dev_states[CANCEL]);
    }
    else if (tempCounter == 0) {
        /* Otherwise if sigproc is done move to COMPLETE state */
        smf_set_state(SMF_CTX(&s_obj), &dev_states[COMPLETE]);
    }

    tempCounter--;
};

void StateMachine::process_exit(void *obj) {

    // Finish signal processing
    LOG_DBG("process exit state");

    /* If the user wants to terminate the test, discard data
    and results */
    if (s_obj.ctx.current == &dev_states[CANCEL]) {
        // discard data, do not send results out
    }
    else {
        // Send results to LCD and potentially to radio handler
    }
};

void StateMachine::complete_entry (void *obj) {
    LOG_DBG("complete entry state");
    LED1::set_solid_green();
};

void StateMachine::complete_run(void *obj) {
    // Display results on LCD
    LOG_DBG("complete run state");

    // Delay
    k_sleep(K_MSEC(1000));

    // Move to IDLE state
    smf_set_state(SMF_CTX(&s_obj), &dev_states[IDLE]);
};

void StateMachine::complete_exit(void *obj) {
    // Move back to IDLE state
    LOG_DBG("complete exit state");
};

void StateMachine::cancel_entry(void *obj) {
    // verify any testing or processing has been stopped
    // verify data has been thrown out data
    LOG_DBG("cancel entry state");
    LED1::set_solid_red();
};

void StateMachine::cancel_run(void *obj) {
    // print error on LCD screen
    LOG_DBG("cancel run state");
    // Delay
    k_sleep(K_MSEC(1000));

    // Move to IDLE state
    smf_set_state(SMF_CTX(&s_obj), &dev_states[IDLE]);
};

void StateMachine::cancel_exit(void *obj) {
    // move back to idle state
    LOG_DBG("cancel exit state");
};

const struct smf_state dev_states[] = {
    [INIT] = SMF_CREATE_STATE(NULL, StateMachine::init_run, NULL),
    [IDLE] = SMF_CREATE_STATE(StateMachine::idle_entry, StateMachine::idle_run, NULL),
    [TEST] = SMF_CREATE_STATE(StateMachine::test_entry, StateMachine::test_run, StateMachine::test_exit),
    [PROCESS] = SMF_CREATE_STATE(StateMachine::process_entry, StateMachine::process_run, StateMachine::process_exit),
    [COMPLETE] = SMF_CREATE_STATE(StateMachine::complete_entry, StateMachine::complete_run, StateMachine::complete_exit),
    [CANCEL] = SMF_CREATE_STATE(StateMachine::cancel_entry, StateMachine::cancel_run, StateMachine::cancel_exit)
};

void TestButton::button_press(const struct device *dev,
                struct gpio_callback *cb, uint32_t pins) {
    /* Generate Button Press Event */
    LOG_DBG("Button press");
    k_event_post(&s_obj.button_press_event, EVENT_BTN_PRESS);
};

void TestButton::init() {
    uint8_t err;

    // Check that button is ready
    if (!gpio_is_ready_dt(&data_enable_button)) {
        LOG_ERR("Error: data_enable_button device %s is not ready\n", data_enable_button.port->name);
        return;
    }

    // Configure button
    err = gpio_pin_configure_dt(&data_enable_button, GPIO_INPUT);
    if (err != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d\n", err, data_enable_button.port->name, data_enable_button.pin);
        return;
    }

    err = gpio_pin_interrupt_configure_dt(&data_enable_button,GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
                err, data_enable_button.port->name, data_enable_button.pin);
        return;
    }

    gpio_init_callback(&TestButton::button_cb_data, TestButton::button_press, BIT(data_enable_button.pin));
    gpio_add_callback(data_enable_button.port, &TestButton::button_cb_data);
    return;
};

void state_machine_init() {	
    uint8_t err;

    LED1::init();
    TestButton::init();

    /* Initialize the event */
    k_event_init(&s_obj.button_press_event);

    /* Set initial state */
    smf_set_initial(SMF_CTX(&s_obj), &dev_states[INIT]);

    /* Run the state machine */
    while(1) {
        /* State machine terminates if a non-zero value is returned */
        err = smf_run_state(SMF_CTX(&s_obj));
        if (err) {
            /* handle return code and terminate state machine */
            break;
        }
        k_msleep(1000);
    }
}
