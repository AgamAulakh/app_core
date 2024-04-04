#include <zephyr/smf.h>
#include <state_machine.h>
#include <led_handler.h>
#include <lcd_handler.h>
#include <HIDThread.h>
#include <Events.h>
#include "DataAcquisitionThread.h"
#include "SignalProcessingThread.h"
#include "drivers/ads1299.h"

LOG_MODULE_REGISTER(state_machine, LOG_LEVEL_DBG);

/* Forward declaration of state table */
extern const struct smf_state dev_states[];

enum dev_state { IDLE, TEST, COMPLETE, CANCEL, DEMO };

/* User defined object */
struct s_object {
    struct smf_ctx ctx;

    /* Events */
    struct k_event sm_event;
    int32_t events;

} s_obj;

void StateMachine::idle_entry(void *obj) {
    LOG_DBG("idle entry state");
    LED1::set_blue();

    // Clear any remaining events if not yet cleared
    if (s_obj.events & EVENT_BTN1_PRESS) {
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_BTN1_PRESS);
    }
    if (s_obj.events & EVENT_BTN2_PRESS) {
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_BTN2_PRESS);
    }
    if (s_obj.events & EVENT_SIG_PROC_COMPLETE) {
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_SIG_PROC_COMPLETE);
    }
    if (s_obj.events & EVENT_RETURN_TO_IDLE) {
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_RETURN_TO_IDLE);
    }
};

void StateMachine::idle_run(void *obj) {
    LOG_DBG("idle run state");

    HIDThread::GetInstance().SendMessage(
        HIDThread::DISPLAY_IDLE
    );

    /* If the button is pressed the user wants to start a test,
    move to TEST state */
    s_obj.events = k_event_wait(&s_obj.sm_event, (EVENT_BTN1_PRESS | EVENT_BTN2_PRESS), true, K_FOREVER);

    // Clear button press
    if (s_obj.events & EVENT_BTN1_PRESS) {
        smf_set_state(SMF_CTX(&s_obj), &dev_states[TEST]);
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_BTN1_PRESS);
    }
    else if (s_obj.events & EVENT_BTN2_PRESS) {
        smf_set_state(SMF_CTX(&s_obj), &dev_states[DEMO]);
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_BTN2_PRESS);
    }
};

void StateMachine::idle_exit(void *obj) {
    // Exit tdle state
    LOG_DBG("idle exit state");
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

    HIDThread::GetInstance().SendMessage(
        HIDThread::DISPLAY_TESTING
    );

    // Have this here a second time as for some reason it does not clear
    // properly when in the previous state?
    s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_BTN1_PRESS);

    // clear msg q of lcd in case a demo result is still there
    // LCD::prepare_queue_for_new_result();

    DataAcquisitionThread::GetInstance().SendMessage(
		DataAcquisitionThread::START_READING_AFE
	);
	SignalProcessingThread::GetInstance().SendMessage(
		SignalProcessingThread::START_PROCESSING
	);

    /* We need to wait for either a button press to cancel or callback from sigproc
    data collection so we know which state to go to */
    s_obj.events = k_event_wait(&s_obj.sm_event, (EVENT_BTN1_PRESS | EVENT_SIG_PROC_COMPLETE), true, K_FOREVER);

    /* If the button is pressed the user wants to terminate
    the test, move to CANCEL state */
    if (s_obj.events & EVENT_BTN1_PRESS) {
        // Currently breaks everything
        SignalProcessingThread::GetInstance().SendMessage(
		    SignalProcessingThread::FORCE_STOP_PROCESSING
	    );
        DataAcquisitionThread::GetInstance().SendMessage(
		    DataAcquisitionThread::STOP_READING_AFE
	    );

        smf_set_state(SMF_CTX(&s_obj), &dev_states[CANCEL]);
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_BTN1_PRESS);
    }
    else {
        DataAcquisitionThread::GetInstance().SendMessage(
		    DataAcquisitionThread::STOP_READING_AFE
	    );

        // Once processing and data collection is done move to COMPLETE state
        smf_set_state(SMF_CTX(&s_obj), &dev_states[COMPLETE]);
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_SIG_PROC_COMPLETE);
    }
};

void StateMachine::test_exit(void *obj) {
    // Stop test - stop data collection
    LOG_DBG("test exit state");

    /* If the user wants to terminate the test, stop
    data processing and discard data */
    if (s_obj.ctx.current == &dev_states[CANCEL]) {
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_BTN1_PRESS);
    }
};

void StateMachine::complete_entry (void *obj) {
    LOG_DBG("complete entry state");
    LED1::set_solid_green();
};

void StateMachine::complete_run(void *obj) {
    // Display results on LCD
    LOG_DBG("complete run state");

    HIDThread::GetInstance().SendMessage(
        HIDThread::DISPLAY_RESULTS
    );

    // Wait to return to idle
    s_obj.events = k_event_wait(&s_obj.sm_event, EVENT_RETURN_TO_IDLE, true, K_FOREVER);
    s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_RETURN_TO_IDLE);
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

    // Cancel does not work right, cannot rerun after cancel
    // device hard faults, data access violation

    s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_BTN1_PRESS);

    HIDThread::GetInstance().SendMessage(
        HIDThread::DISPLAY_CANCEL
    );

    // Delay
    k_sleep(K_MSEC(1000));

    // Move to IDLE state
    smf_set_state(SMF_CTX(&s_obj), &dev_states[IDLE]);
};

void StateMachine::cancel_exit(void *obj) {
    // move back to idle state
    LOG_DBG("cancel exit state");
};

void StateMachine::demo_entry(void *obj) {
    LOG_DBG("demo entry state");
    LED1::set_solid_purple();

    HIDThread::GetInstance().SendMessage(
        HIDThread::DISPLAY_DEMO
    );
};

void StateMachine::demo_run(void *obj) {
    LOG_DBG("demo run state");

    // LCD::prepare_queue_for_new_result();

    DataAcquisitionThread::GetInstance().SendMessage(
		DataAcquisitionThread::RUN_INTERNAL_SQUARE_WAVE_TEST_BIG_FAST
	);
	SignalProcessingThread::GetInstance().SendMessage(
		SignalProcessingThread::START_PROCESSING
	);

    s_obj.events = k_event_wait(&s_obj.sm_event, EVENT_SIG_PROC_COMPLETE, true, K_FOREVER);
    s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_SIG_PROC_COMPLETE); 

	DataAcquisitionThread::GetInstance().SendMessage(
		DataAcquisitionThread::STOP_READING_AFE
	);

    // sleep for 2 seconds
	k_msleep(2000);

    HIDThread::GetInstance().SendMessage(
        HIDThread::DISPLAY_RESULTS
    );

    // Wait for results to finish being displayed
    s_obj.events = k_event_wait(&s_obj.sm_event, EVENT_RETURN_TO_IDLE, true, K_FOREVER);
    s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_RETURN_TO_IDLE);

    
    HIDThread::GetInstance().SendMessage(
        HIDThread::DISPLAY_DEMO
    );

    // LCD::prepare_queue_for_new_result();

    // sleep for 2 seconds
	k_msleep(2000);

    DataAcquisitionThread::GetInstance().SendMessage(
		DataAcquisitionThread::RUN_INTERNAL_SQUARE_WAVE_TEST_SMALL_SLOW
	);
	SignalProcessingThread::GetInstance().SendMessage(
		SignalProcessingThread::START_PROCESSING
	);

    s_obj.events = k_event_wait(&s_obj.sm_event, EVENT_SIG_PROC_COMPLETE, true, K_FOREVER);
    s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_SIG_PROC_COMPLETE);

	DataAcquisitionThread::GetInstance().SendMessage(
		DataAcquisitionThread::STOP_READING_AFE
	);

    // sleep for 2 seconds
	k_msleep(2000);

    HIDThread::GetInstance().SendMessage(
        HIDThread::DISPLAY_RESULTS
    );

    // Wait for results to finish being displayed
    s_obj.events = k_event_wait(&s_obj.sm_event, EVENT_RETURN_TO_IDLE, true, K_FOREVER);
    s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_RETURN_TO_IDLE);

    HIDThread::GetInstance().SendMessage(
        HIDThread::DISPLAY_DEMO
    );

    /* If button 1 is pressed the user wants to start a demo 
       If button 2 is pressed return to IDLE state */
    s_obj.events = k_event_wait(&s_obj.sm_event, (EVENT_BTN1_PRESS | EVENT_BTN2_PRESS), true, K_FOREVER);

    // Clear button press
    if (s_obj.events & EVENT_BTN1_PRESS) {
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_BTN1_PRESS);
        smf_set_state(SMF_CTX(&s_obj), &dev_states[IDLE]);
    }
    else if (s_obj.events & EVENT_BTN2_PRESS) {
        s_obj.events = k_event_clear(&s_obj.sm_event, EVENT_BTN2_PRESS);
    }
};

void StateMachine::demo_exit(void *obj) {
    // move back to idle state
    LOG_DBG("demo exit state");
};

const struct smf_state dev_states[] = {
    [IDLE] = SMF_CREATE_STATE(StateMachine::idle_entry, StateMachine::idle_run, StateMachine::idle_exit),
    [TEST] = SMF_CREATE_STATE(StateMachine::test_entry, StateMachine::test_run, StateMachine::test_exit),
    [COMPLETE] = SMF_CREATE_STATE(StateMachine::complete_entry, StateMachine::complete_run, StateMachine::complete_exit),
    [CANCEL] = SMF_CREATE_STATE(StateMachine::cancel_entry, StateMachine::cancel_run, StateMachine::cancel_exit),
    [DEMO] =  SMF_CREATE_STATE(StateMachine::demo_entry, StateMachine::demo_run, StateMachine::demo_exit)
};

void sig_proc_complete(void) {
    /* Generate Sig Proc Complete Event */
    LOG_DBG("Sig proc event");
    k_event_post(&s_obj.sm_event, EVENT_SIG_PROC_COMPLETE);
}

void return_to_idle(void) {
    /* Generate Return to Idle Event if user is done viewing results */
    LOG_DBG("Sig proc event");
    k_event_post(&s_obj.sm_event, EVENT_RETURN_TO_IDLE);
}

void Button1::button_press(const struct device *dev,
                struct gpio_callback *cb, uint32_t pins) {
    /* Generate Button Press Event */
    LOG_DBG("Button 1 press");
    k_event_post(&s_obj.sm_event, EVENT_BTN1_PRESS);
};

void Button2::button_press(const struct device *dev,
                struct gpio_callback *cb, uint32_t pins) {
    /* Generate Button Press Event */
    LOG_DBG("Button 2 press");
    k_event_post(&s_obj.sm_event, EVENT_BTN2_PRESS);
};

void Button1::init() {
    uint8_t err;

    // Check that button is ready
    if (!gpio_is_ready_dt(&button1)) {
        LOG_ERR("Error: button1 device %s is not ready\n", button1.port->name);
        return;
    }

    // Configure button
    err = gpio_pin_configure_dt(&button1, GPIO_INPUT);
    if (err != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d\n", err, button1.port->name, button1.pin);
        return;
    }

    err = gpio_pin_interrupt_configure_dt(&button1, GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
                err, button1.port->name, button1.pin);
        return;
    }

    gpio_init_callback(&Button1::button_cb_data, Button1::button_press, BIT(button1.pin));
    gpio_add_callback(button1.port, &Button1::button_cb_data);
    return;
};

void Button2::init() {
    uint8_t err;

    // Check that button is ready
    if (!gpio_is_ready_dt(&button2)) {
        LOG_ERR("Error: button1 device %s is not ready\n", button2.port->name);
        return;
    }

    // Configure button
    err = gpio_pin_configure_dt(&button2, GPIO_INPUT);
    if (err != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d\n", err, button2.port->name, button2.pin);
        return;
    }

    err = gpio_pin_interrupt_configure_dt(&button2, GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
                err, button2.port->name, button2.pin);
        return;
    }

    gpio_init_callback(&Button2::button_cb_data, Button2::button_press, BIT(button2.pin));
    gpio_add_callback(button2.port, &Button2::button_cb_data);
    return;
};

void state_machine_init() {	
    uint8_t err;

    LOG_DBG("State Machine Initialization");

    /* Initalize namespaces */
    LED1::init();
    Button1::init();
    Button2::init();

    /* Initialize the event */
    k_event_init(&s_obj.sm_event);
    k_event_init(&s_obj.sm_event);

    /* Set initial state */
    smf_set_initial(SMF_CTX(&s_obj), &dev_states[IDLE]);

    HIDThread::GetInstance().SendMessage(
        HIDThread::DISPLAY_INIT
    );

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
