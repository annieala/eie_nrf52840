/*
 * main.c
 */

#include <zephyr/kernel.h> 
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h> 

#define SLEEP_TIME_MS 50

#define LED0_NODE DT_ALIAS(led0)
#define SW0_NODE DT_ALIAS(sw0)
#define SW1_NODE DT_ALIAS(sw1)
#define SW2_NODE DT_ALIAS(sw2)
#define SW3_NODE DT_ALIAS(sw3)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(SW3_NODE, gpios);

// Password: 1111 (BTN1 four times)
#define PASSWORD_LENGTH 4
static const uint8_t correct_password[PASSWORD_LENGTH] = {0, 0, 0, 0};

// States
typedef enum {
  STATE_LOCKED,
  STATE_WAITING
} system_state_t;

static system_state_t current_state = STATE_LOCKED;
static uint8_t password_buffer[PASSWORD_LENGTH];
static uint8_t password_index = 0;

// Button tracking
static bool btn0_was_pressed = false;
static bool btn1_was_pressed = false;
static bool btn2_was_pressed = false;
static bool btn3_was_pressed = false;

void reset_password_entry(void) {
  password_index = 0;
  for(int i = 0; i < PASSWORD_LENGTH; i++) {
    password_buffer[i] = 0xFF;
  }
}

bool check_password(void) {
  if(password_index != PASSWORD_LENGTH) {
    return false;
  }
  
  for(int i = 0; i < PASSWORD_LENGTH; i++) {
    if(password_buffer[i] != correct_password[i]) {
      return false;
    }
  }
  return true;
}

int main(void) {
  int ret;

  // Check if LED is ready
  if(!gpio_is_ready_dt(&led0)) {
    return 0;
  }

  // Check if buttons are ready
  if(!gpio_is_ready_dt(&button0) || !gpio_is_ready_dt(&button1) ||
     !gpio_is_ready_dt(&button2) || !gpio_is_ready_dt(&button3)) {
    return 0;
  }

  // Configure LED0 as output (ON = locked state)
  ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
  if(0 > ret) {
    return 0;
  }

  // Configure all buttons as inputs
  ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
  if(0 > ret) {
    return 0;
  }

  ret = gpio_pin_configure_dt(&button1, GPIO_INPUT);
  if(0 > ret) {
    return 0;
  }

  ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
  if(0 > ret) {
    return 0;
  }

  ret = gpio_pin_configure_dt(&button3, GPIO_INPUT);
  if(0 > ret) {
    return 0;
  }

  printk("Password system started\n");
  printk("Correct password: BTN1, BTN1, BTN1, BTN1\n");
  
  reset_password_entry();

  while(1) {
    // Read button states
    int btn0_state = gpio_pin_get_dt(&button0);
    int btn1_state = gpio_pin_get_dt(&button1);
    int btn2_state = gpio_pin_get_dt(&button2);
    int btn3_state = gpio_pin_get_dt(&button3);

    if(current_state == STATE_LOCKED) {
      // BTN0 pressed
      if(0 < btn0_state) {
        if(!btn0_was_pressed && password_index < PASSWORD_LENGTH) {
          password_buffer[password_index] = 0;
          password_index++;
          printk("Entered: BTN0\n");
          btn0_was_pressed = true;
        }
      } else {
        btn0_was_pressed = false;
      }

      // BTN1 pressed
      if(0 < btn1_state) {
        if(!btn1_was_pressed && password_index < PASSWORD_LENGTH) {
          password_buffer[password_index] = 1;
          password_index++;
          printk("Entered: BTN1\n");
          btn1_was_pressed = true;
        }
      } else {
        btn1_was_pressed = false;
      }

      // BTN2 pressed
      if(0 < btn2_state) {
        if(!btn2_was_pressed && password_index < PASSWORD_LENGTH) {
          password_buffer[password_index] = 2;
          password_index++;
          printk("Entered: BTN2\n");
          btn2_was_pressed = true;
        }
      } else {
        btn2_was_pressed = false;
      }

      // BTN3 pressed (enter button)
      if(0 < btn3_state) {
        if(!btn3_was_pressed) {
          if(check_password()) {
            printk("Correct!\n");
          } else {
            printk("Incorrect!\n");
          }
          
          gpio_pin_set_dt(&led0, 0);  // Turn LED0 off
          current_state = STATE_WAITING;
          reset_password_entry();
          btn3_was_pressed = true;
        }
      } else {
        btn3_was_pressed = false;
      }
    }
    else if(current_state == STATE_WAITING) {
      // Any button press resets to locked state
      if(0 < btn0_state && !btn0_was_pressed) {
        printk("Resetting to locked state\n");
        current_state = STATE_LOCKED;
        gpio_pin_set_dt(&led0, 1);
        reset_password_entry();
        btn0_was_pressed = true;
      } else if(btn0_state == 0) {
        btn0_was_pressed = false;
      }

      if(0 < btn1_state && !btn1_was_pressed) {
        printk("Resetting to locked state\n");
        current_state = STATE_LOCKED;
        gpio_pin_set_dt(&led0, 1);
        reset_password_entry();
        btn1_was_pressed = true;
      } else if(btn1_state == 0) {
        btn1_was_pressed = false;
      }

      if(0 < btn2_state && !btn2_was_pressed) {
        printk("Resetting to locked state\n");
        current_state = STATE_LOCKED;
        gpio_pin_set_dt(&led0, 1);
        reset_password_entry();
        btn2_was_pressed = true;
      } else if(btn2_state == 0) {
        btn2_was_pressed = false;
      }

      if(0 < btn3_state && !btn3_was_pressed) {
        printk("Resetting to locked state\n");
        current_state = STATE_LOCKED;
        gpio_pin_set_dt(&led0, 1);
        reset_password_entry();
        btn3_was_pressed = true;
      } else if(btn3_state == 0) {
        btn3_was_pressed = false;
      }
    }

    k_msleep(SLEEP_TIME_MS);
  }

  return 0;
}