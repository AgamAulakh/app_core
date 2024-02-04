#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>
#include <led_handler.h>

LOG_MODULE_REGISTER(state_machine, LOG_LEVEL_DBG);

#define THREAD_STATE_SIZE 1028 // arbitrary for now

#define Button1 DT_ALIAS(sw0) // Button for initiating or cancelling test
#if !DT_NODE_HAS_STATUS(Button1, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

/* List of events */
#define EVENT_BTN_PRESS BIT(0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(Button1, gpios, {0});
static struct gpio_callback button_cb_data;

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

static void init_run(void *obj) {
    // Setup threads
    LOG_DBG("init run state");

    smf_set_state(SMF_CTX(&s_obj), &dev_states[IDLE]);
}

static void idle_entry(void *obj) {
    LOG_DBG("idle entry state");
    // Set LED2 to blue
}

static void idle_run(void *obj) {
    LOG_DBG("idle run state");

    /* If the button is pressed the user wants to start a test,
    move to TEST state */
    s_obj.events = k_event_wait(&s_obj.button_press_event, EVENT_BTN_PRESS, true, K_FOREVER);

    if (s_obj.events & EVENT_BTN_PRESS) {
        smf_set_state(SMF_CTX(&s_obj), &dev_states[TEST]);
        s_obj.events = k_event_clear(&s_obj.button_press_event, EVENT_BTN_PRESS);
    }
}

static void test_entry(void *obj) {
    // Set LED2 to yellow
    LOG_DBG("test entry state");
}

static void test_run(void *obj) {
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
}

static void test_exit(void *obj) {
    // Stop test - stop data collection
    LOG_DBG("test exit state");

    /* If the user wants to terminate the test, stop
    data processing and discard data */
    if (s_obj.ctx.current == &dev_states[CANCEL]) {
        LOG_DBG("Current state? %d", s_obj.ctx.current);
        // stop data processing
    }
}

static void process_entry(void *obj) {
    // Set LED2 to flashing green
    LOG_DBG("process entry state");
}

static void process_run(void *obj) {
    // Run signal processing
    LOG_DBG("process run state");

    /* We need to wait for either a button press to cancel or callback from sigproc
    data collection so we know which state to go to */
    // s_obj.events = k_event_wait(&s_obj.button_press_event, EVENT_BTN_PRESS, true, K_FOREVER);

    if (s_obj.events & EVENT_BTN_PRESS) {
        /* If the button is pressed the user wants to terminate
        the test, move to CANCEL state */
        smf_set_state(SMF_CTX(&s_obj), &dev_states[CANCEL]);
    }
    else {
        /* Otherwise if sigproc is done move to COMPLETE state */
        smf_set_state(SMF_CTX(&s_obj), &dev_states[COMPLETE]);
    }
}

static void process_exit(void *obj) {
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
}

static void complete_entry (void *obj) {
    // Set LED2 to solid green
    LOG_DBG("complete entry state");
}

static void complete_run(void *obj) {
    // Display results on LCD
    LOG_DBG("complete run state");
    // Delay
    k_sleep(K_MSEC(1000));

    // Move to IDLE state
    smf_set_state(SMF_CTX(&s_obj), &dev_states[IDLE]);
}

static void complete_exit(void *obj) {
    // Move back to idle
    LOG_DBG("complete exit state");
}

static void cancel_entry(void *obj) {
    // verify any testing or processing has been stopped
    // verify data has been thrown out data
    // Set LED2 to solid red
    LOG_DBG("cancel entry state");
}

static void cancel_run(void *obj) {
    // print error on LCD screen
    LOG_DBG("cancel run state");
    // Delay
    k_sleep(K_MSEC(1000));

    // Move to IDLE state
    smf_set_state(SMF_CTX(&s_obj), &dev_states[IDLE]);
}

static void cancel_exit(void *obj) {
    // move back to idle state
    LOG_DBG("cancel exit state");
}

const struct smf_state dev_states[] = {
    [INIT] = SMF_CREATE_STATE(NULL, init_run, NULL),
    [IDLE] = SMF_CREATE_STATE(idle_entry, idle_run, NULL),
    [TEST] = SMF_CREATE_STATE(test_entry, test_run, test_exit),
    [PROCESS] = SMF_CREATE_STATE(process_entry, process_run, process_exit),
    [COMPLETE] = SMF_CREATE_STATE(complete_entry, complete_run, complete_exit),
    [CANCEL] = SMF_CREATE_STATE(cancel_entry, cancel_run, cancel_exit)
};

void button_press(const struct device *dev,
                struct gpio_callback *cb, uint32_t pins)
{
    /* Generate Button Press Event */
    LOG_DBG("Button press");
    k_event_post(&s_obj.button_press_event, EVENT_BTN_PRESS);
}

void button_init() {
    uint8_t err;

    // Check that button is ready
    if (!gpio_is_ready_dt(&button)) {
        LOG_ERR("Error: button device %s is not ready\n", button.port->name);
        return;
    }

    // Configure button
    err = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (err != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d\n", err, button.port->name, button.pin);
        return;
    }

    err = gpio_pin_interrupt_configure_dt(&button,GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
                err, button.port->name, button.pin);
        return;
    }

    gpio_init_callback(&button_cb_data, button_press, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    return;
}

void state_machine_init()
{	
    uint8_t err;

    button_init();

    led_init();

    /* Initialize the event */
    k_event_init(&s_obj.button_press_event);

    /* Set initial state */
    smf_set_initial(SMF_CTX(&s_obj), &dev_states[INIT]);

    /* Run the state machine */
    while(1) {
        // if (s_obj.ctx.current == &dev_states[IDLE]) {
        //     /* Sit in IDLE state until a button event is detected */
        //     s_obj.events = k_event_wait(&s_obj.button_press_event, EVENT_BTN_PRESS, true, K_FOREVER);
        // }

        /* State machine terminates if a non-zero value is returned */
        err = smf_run_state(SMF_CTX(&s_obj));
        if (err) {
            /* handle return code and terminate state machine */
            break;
        }
        k_msleep(1000);
    }
}
