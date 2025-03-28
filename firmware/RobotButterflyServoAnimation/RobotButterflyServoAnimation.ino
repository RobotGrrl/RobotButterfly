/*
 * Robot Butterfly Servo Animation
 * --------------------------------
 * Servo animations
 * 
 * Board: ESP32 Dev Module
 * CPU speed: 80 MHz
 * Upload speed: 115200 baud
 * esp boards package: v 3.0.2
 *
 * 18 Mar 2025 - Erin RobotZwrrl for Robot Missions Inc
 * http://robotzwrrl.xyz/robot-butterfly
 * http://robotmissions.org
 */

#include "Board.h"
#include "Params.h"
#include <Streaming.h>
#include <ESP32Servo.h>      // 3.0.6 https://github.com/madhephaestus/ESP32Servo 
#include "ServoEasing.hpp"   // 3.4.0 https://github.com/ArminJo/ServoEasing



// ----------- servo motors -----------
ServoEasing s1;
ServoEasing s2;

struct ServoMotor {
  ServoEasing motor;
  uint16_t home_pos;
  uint16_t up_pos;
  uint16_t down_pos;
  uint16_t current_pos;
  uint16_t target_pos;
  bool active;
  bool calibrated;
  String name;
};

struct ServoMotor wing_left;
struct ServoMotor wing_right;

bool initialised_servos = false;
// -----------------------------------


// ----------- servo animation -----------
enum servoAnimName {
  SERVO_ANIM_NONE,
  SERVO_ANIM_GENTLE,
  SERVO_ANIM_SWAY,
  SERVO_ANIM_SOARING,
  SERVO_ANIM_TOUCHGRASS,
  SERVO_ANIM_SWOOSH,
  SERVO_ANIM_PARTY,
  SERVO_ANIM_FLUTTER,
  SERVO_ANIM_RANGE,
  SERVO_ANIM_POSITION
};

enum servoAnimType {
  SERVO_ANIM_ALERT,
  SERVO_ANIM_HOME
};

void runServoAnim_none(struct ServoAnimation *animation);
void runServoAnim_gentle(struct ServoAnimation *animation);
void runServoAnim_sway(struct ServoAnimation *animation);
void runServoAnim_soaring(struct ServoAnimation *animation);
void runServoAnim_touchgrass(struct ServoAnimation *animation);
void runServoAnim_swoosh(struct ServoAnimation *animation);
void runServoAnim_party(struct ServoAnimation *animation);
void runServoAnim_flutter(struct ServoAnimation *animation);
void runServoAnim_range(struct ServoAnimation *animation);
void runServoAnim_position(struct ServoAnimation *animation);
typedef void (*ServoAnimationFunction)(ServoAnimation*); // function pointer type that accepts a ServoAnimation pointer

struct ServoAnimation {
  uint8_t id;
  bool active;
  uint8_t type;
  uint8_t velocity;   // degrees per second

  uint8_t num_frames;
  uint16_t frame_delay;
  int frame_index;
  long last_frame;
  
  int num_repeats;
  uint16_t repeat_count;
  uint16_t repeat_delay;
  long last_repeat;

  long duration;
  long start_time;

  bool dir;
  int helper1;
  int helper2;
  int helper3;

  ServoAnimationFunction function;  // function pointer
};

ServoAnimation servo_animation_home;
ServoAnimation servo_animation_alert;
// -----------------------------------


long last_print = 0;
uint8_t ranger = 0;


void setup() {
  Serial.begin(9600);

  print_wakeup_reason();

  initServos();
  initServoAnimations();

  Serial << "Ready" << endl;
}


void loop() {
  
  if(millis()-last_print >= 1000) {
    //Serial << millis() << " hi " << xPortGetCoreID() << endl;
    if(servo_animation_alert.id == SERVO_ANIM_RANGE) {
      ranger++;
      if(ranger > 9) ranger = 0;
      servo_animation_alert.helper2 = ranger;
    }
    last_print = millis();
  }

  updateServoAnimation();

  if(Serial.available()) {
    char c = Serial.read();
    switch(c) {
      case '0':
        setServoAnim(&servo_animation_alert, SERVO_ANIM_NONE, SERVO_ANIM_ALERT);
        startServoAnim(&servo_animation_alert);
      break;
      case '1':
        Serial << "Gentle default" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_GENTLE, SERVO_ANIM_ALERT);
        startServoAnim(&servo_animation_alert);
      break;
      case '2':
        Serial << "Sway default" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_SWAY, SERVO_ANIM_ALERT);
        startServoAnim(&servo_animation_alert);
      break;
      case '3':
        Serial << "Gentle 5 seconds and faster velocity" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_GENTLE, SERVO_ANIM_ALERT);
        setServoAnimDuration(&servo_animation_alert, 5000);
        setServoAnimSpeed(&servo_animation_alert, 80);
        startServoAnim(&servo_animation_alert);
      break;
      case '4':
        Serial << "Sway 5 seconds and faster velocity" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_SWAY, SERVO_ANIM_ALERT);
        setServoAnimDuration(&servo_animation_alert, 5000);
        setServoAnimSpeed(&servo_animation_alert, 100);
        startServoAnim(&servo_animation_alert);
      break;
      case '5':
        Serial << "Gentle 3x" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_GENTLE, SERVO_ANIM_ALERT);
        setServoAnimRepeats(&servo_animation_alert, 3);
        startServoAnim(&servo_animation_alert);
      break;
      case '6':
        Serial << "Sway 3x" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_SWAY, SERVO_ANIM_ALERT);
        setServoAnimRepeats(&servo_animation_alert, 3);
        startServoAnim(&servo_animation_alert);
      break;
      case '7':
        Serial << "Soaring (default)" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_SOARING, SERVO_ANIM_ALERT);
        startServoAnim(&servo_animation_alert);
      break;
      case '8':
        Serial << "Soaring faster velocity" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_SOARING, SERVO_ANIM_ALERT);
        setServoAnimSpeed(&servo_animation_alert, 200);
        startServoAnim(&servo_animation_alert);
      break;
      case '9':
        Serial << "Touch grass (default)" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_TOUCHGRASS, SERVO_ANIM_ALERT);
        startServoAnim(&servo_animation_alert);
      break;
      case 'q':
        Serial << "Swoosh" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_SWOOSH, SERVO_ANIM_ALERT);
        startServoAnim(&servo_animation_alert);
      break;
      case 'w':
        Serial << "Swoosh 5 seconds" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_SWOOSH, SERVO_ANIM_ALERT);
        setServoAnimDuration(&servo_animation_alert, 5000);
        startServoAnim(&servo_animation_alert);
      break;
      case 'e':
        Serial << "Swoosh 5 seconds and fast velocity" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_SWOOSH, SERVO_ANIM_ALERT);
        setServoAnimDuration(&servo_animation_alert, 5000);
        setServoAnimSpeed(&servo_animation_alert, 100);
        startServoAnim(&servo_animation_alert);
      break;
      case 'r':
        Serial << "Party" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_PARTY, SERVO_ANIM_ALERT);
        startServoAnim(&servo_animation_alert);
      break;
      case 't':
        Serial << "Party 5 seconds and fast velocity" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_PARTY, SERVO_ANIM_ALERT);
        setServoAnimDuration(&servo_animation_alert, 5000);
        setServoAnimSpeed(&servo_animation_alert, 200);
        startServoAnim(&servo_animation_alert);
      break;
      case 'y':
        Serial << "Flutter left home, right up" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_FLUTTER, SERVO_ANIM_ALERT);
        servo_animation_alert.helper1 = 2;
        servo_animation_alert.helper2 = 2;
        startServoAnim(&servo_animation_alert);
      break;
      case 'u':
        Serial << "Flutter home" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_FLUTTER, SERVO_ANIM_ALERT);
        servo_animation_alert.helper1 = 0;
        startServoAnim(&servo_animation_alert);
      break;
      case 'i':
        Serial << "Flutter right up, left up" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_FLUTTER, SERVO_ANIM_ALERT);
        servo_animation_alert.helper1 = 5;
        servo_animation_alert.helper2 = 2;
        startServoAnim(&servo_animation_alert);
      break;
      case 'o':
        Serial << "Range down to up" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_RANGE, SERVO_ANIM_ALERT);
        servo_animation_alert.helper1 = 0;
        servo_animation_alert.helper2 = ranger;
        startServoAnim(&servo_animation_alert);
      break;
      case 'p':
        Serial << "Range home to up" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_RANGE, SERVO_ANIM_ALERT);
        servo_animation_alert.helper1 = 1;
        servo_animation_alert.helper2 = ranger;
        startServoAnim(&servo_animation_alert);
      break;
      case '[':
        Serial << "Range alt" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_RANGE, SERVO_ANIM_ALERT);
        servo_animation_alert.helper1 = 2;
        servo_animation_alert.helper2 = ranger;
        startServoAnim(&servo_animation_alert);
      break;
      case 'g':
        Serial << "Position" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_POSITION, SERVO_ANIM_ALERT);
        servo_animation_alert.helper1 = 1;
        servo_animation_alert.helper2 = 1;
        startServoAnim(&servo_animation_alert);
      break;
      case 'j':
        Serial << "Position" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_POSITION, SERVO_ANIM_ALERT);
        servo_animation_alert.helper1 = 2;
        servo_animation_alert.helper2 = 1;
        startServoAnim(&servo_animation_alert);
      break;
      case 'k':
        Serial << "Position" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_POSITION, SERVO_ANIM_ALERT);
        servo_animation_alert.helper1 = 2;
        servo_animation_alert.helper2 = 2;
        startServoAnim(&servo_animation_alert);
      break;
      case 'l':
        Serial << "Position" << endl;
        setServoAnim(&servo_animation_alert, SERVO_ANIM_POSITION, SERVO_ANIM_ALERT);
        servo_animation_alert.helper1 = 1;
        servo_animation_alert.helper2 = 0;
        startServoAnim(&servo_animation_alert);
      break;
      case 's':
        stopServoAnim(&servo_animation_alert);
      break;
      case 'h':
        Serial << "1-9: animations" << endl;
        Serial << "s: stop" << endl;
        Serial << "h: help" << endl;
      break;
    }
  }

}

