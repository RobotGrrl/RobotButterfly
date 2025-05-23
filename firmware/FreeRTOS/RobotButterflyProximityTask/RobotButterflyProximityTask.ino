/*
 * Robot Butterfly Proximity
 * ---------------------------
 * Measure distance with the HC-SR04 sensor
 * 
 * Board: ESP32 Dev Module
 * CPU speed: 80 MHz
 * Upload speed: 115200 baud
 * esp boards package: v 3.0.2
 *
 * 27 Mar 2025 - Erin RobotZwrrl for Robot Missions Inc
 * http://robotzwrrl.xyz/robot-butterfly
 * http://robotmissions.org
 */

// @module: Proximity
// @version: 0.1.0

#include "Board.h"
#include "Params.h"
#include "ParamsRTOS.h"
#include <Streaming.h>
#include <movingAvg.h>   // 2.3.2 https://github.com/JChristensen/movingAvg
#include <HCSR04.h>      // 1.1.3 https://github.com/d03n3rfr1tz3/HC-SR04 


// ------------ callbacks ------------
void proximityTriggerCallback(struct Proximity *p);
typedef void (*ProximityCallback)(struct Proximity *p); // proximity callback type
// ------------------------------------

// ----------- ultrasonic -------------
struct Proximity {
  bool initialised;
  long last_print;
  long last_trigger;
  uint8_t threshold;

  volatile bool update_raw;
  volatile uint8_t iteration_raw;
  volatile uint8_t reload_raw;
  uint16_t raw;
  uint16_t raw_prev;
  long last_raw;

  uint16_t val;
  uint16_t val_prev;
  movingAvg val_avg;

  ProximityCallback onProximityTriggerCallback;

  Proximity() 
  : initialised(false), last_print(0), last_trigger(0), threshold(0),
    update_raw(false), iteration_raw(0), reload_raw(0), raw(0), raw_prev(0), last_raw(0),
    val_avg(PROXIMITY_MOVING_AVG_WINDOW, true), onProximityTriggerCallback(NULL)
{}

};

static struct Proximity ultrasonic;

hw_timer_t *timer_10Hz_config = NULL;
// ------------------------------------

// ---------- proximity isr ----------
void IRAM_ATTR Timer_10Hz_ISR() { // every 0.1 seconds

  // refresh ultrasonic every 0.5 seconds
  if(ultrasonic.iteration_raw >= ultrasonic.reload_raw-1) {
    ultrasonic.update_raw = true;
    ultrasonic.iteration_raw = 0;
  } else {
    if(ultrasonic.update_raw == false) {
      ultrasonic.iteration_raw++;
    }
  }

}
// ------------------------------------

// ---------- rtos ------------
static TaskHandle_t Task_PROXIMITY;
static SemaphoreHandle_t Mutex_PROXIMITY;
long last_proximity_rtos_print = 0;
// -------------------------------


void setup() {
  Serial.begin(9600);
  
  print_wakeup_reason();

  initProximity();

  Serial << "Ready" << endl;
}


void loop() {
  
  //updateProximity();

  commandLine();

}


void commandLine() {
  if(Serial.available()) {
    char c = Serial.read();
    switch(c) {
      case 'h':
        Serial << "h: help" << endl;
      break;
    }
  }
}


