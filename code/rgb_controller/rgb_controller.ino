/**
 * @file rgb_controller.ino
 * 
 * @author Joseph Telaak
 * 
 * @brief RGB controller module to control 3 independent LED light strips via the CAN bus
 * 
 * @version 0.1
 * 
 * @date 2022-08-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

// CAN Setup
#define CAN_CS 4    // CAN SPI Chip Select pin
#define CAN_INT 2   // CAN SPI Interrupt pin

#define CAN_DLC 8   // CAN data length

#define CAN_ID 0x00 // CAN ID

#include <mcp2515.h>    // CAN library

MCP2515 can(CAN_CS, CAN_INT);   // CAN library object

// LED Load Measure
#define LED_LOAD_MEASURE A0

// LED Controller 1
#define LED_CONTROLLER_1_R 10
#define LED_CONTROLLER_1_G 9
#define LED_CONTROLLER_1_B 6

// LED Controller 2
#define LED_CONTROLLER_2_R 8
#define LED_CONTROLLER_2_G 7
#define LED_CONTROLLER_2_B 5

// LED Controller 3
#define LED_CONTROLLER_3_R A3
#define LED_CONTROLLER_3_G A2
#define LED_CONTROLLER_3_B A1

// Arduino setup function
void setup() {
    // LED Controller 1 Output Pins
    pinMode(LED_CONTROLLER_1_R, OUTPUT);
    pinMode(LED_CONTROLLER_1_G, OUTPUT);
    pinMode(LED_CONTROLLER_1_B, OUTPUT);

    // LED Controller 2 Output Pins
    pinMode(LED_CONTROLLER_2_R, OUTPUT);
    pinMode(LED_CONTROLLER_2_G, OUTPUT);
    pinMode(LED_CONTROLLER_2_B, OUTPUT);

    // LED Controller 3 Output Pins
    pinMode(LED_CONTROLLER_3_R, OUTPUT);
    pinMode(LED_CONTROLLER_3_G, OUTPUT);
    pinMode(LED_CONTROLLER_3_B, OUTPUT);

    // LED Load Measure Input Pin
    pinMode(LED_LOAD_MEASURE, INPUT);

    // Setup CAN
    can.reset();
    can.setBitrate(CAN_125KBPS);
    can.setNormalMode();

    // Blink to indicate startup
    blink(4);
    
    // Attach interrupt to CAN interrupt pin
    attachInterrupt(digitalPinToInterrupt(CAN_INT), can_interrupt, FALLING);

}

// Arduino loop function
void loop() {
    // Report LED states and load measurement every 5 seconds
    if (millis() - last_report > 5000) {
        last_report = millis();
        report_led_states();
        report_load_measurement();

    }
}

// Can interrupt function
void can_interrupt() {
    // Read CAN message
    can.read(rx_msg);
    
    // Check if message has correct ID
    if (rx_msg.id == CAN_ID) {

        // Check the 2nd byte to determine control mode
        switch (rx_msg.data[1]) {
            case 0: // Turn off all LEDs
                turn_off_all_leds();
                break;

            case 1: // Set Static States
                set_static_states(rx_msg.data[2], rx_msg.data[5], rx_msg.data[6], rx_msg.data[7]);
                break;

            case 2: // Blink LEDs
                blink(rx_msg.data[2],rx_msg.data[3]);
                break;

            case 3: // Run rgb light show
                rgb_light_show();
                break;

            case 4: // Report the load measure
                report_led_load();
                break;

            case 5: // Report the current states of the LEDs
                report_values();
                break;
            
        }
    }
}

// Report the LED load to the CAN bus
void report_led_load() {
    // Read the LED load
    int led_load = analogRead(LED_LOAD_MEASURE);
    
    // Create CAN message
    tx_msg.id = CAN_ID;
    tx_msg.length = CAN_DLC;
    tx_msg.data[0] = 0x00;
    tx_msg.data[1] = 0x00;
    tx_msg.data[2] = 0x00;
    tx_msg.data[3] = 0x00;
    tx_msg.data[4] = 0x00;
    tx_msg.data[5] = 0x00;
    tx_msg.data[6] = 0x00;
    tx_msg.data[7] = 0x00;
    
    // Set the data bytes
    tx_msg.data[1] = 0xFF;
    tx_msg.data[6] = led_load >> 8;
    tx_msg.data[7] = led_load;
    
    // Send the CAN message
    can.write(tx_msg);

}

// Report current LED controller values to the CAN bus scaled to 0-255
void report_values() {
    // Create CAN message
    tx_msg.id = CAN_ID;
    tx_msg.length = CAN_DLC;
    tx_msg.data[0] = 0x00;
    tx_msg.data[1] = 0x00;
    tx_msg.data[2] = 0x00;
    tx_msg.data[3] = 0x00;
    tx_msg.data[4] = 0x00;
    tx_msg.data[5] = 0x00;
    tx_msg.data[6] = 0x00;
    tx_msg.data[7] = 0x00;

    // Scale the values to 0-255
    int red = map(analogRead(LED_CONTROLLER_1_R), 0, 1023, 0, 255);
    int green = map(analogRead(LED_CONTROLLER_1_G), 0, 1023, 0, 255);
    int blue = map(analogRead(LED_CONTROLLER_1_B), 0, 1023, 0, 255);

    // Set the data bytes
    tx_msg.data[1] = 0xFE;
    tx_msg.data[2] = 1;
    tx_msg.data[5] = red;
    tx_msg.data[6] = green;
    tx_msg.data[7] = blue;

    // Send the CAN message
    can.write(tx_msg);

    // Scale the values to 0-255
    int red = map(analogRead(LED_CONTROLLER_2_R), 0, 1023, 0, 255);
    int green = map(analogRead(LED_CONTROLLER_2_G), 0, 1023, 0, 255);
    int blue = map(analogRead(LED_CONTROLLER_2_B), 0, 1023, 0, 255);

    // Set the data bytes
    tx_msg.data[1] = 0xFE;
    tx_msg.data[2] = 2;
    tx_msg.data[5] = red;
    tx_msg.data[6] = green;
    tx_msg.data[7] = blue;

    // Send the CAN message
    can.write(tx_msg);

    // Scale the values to 0-255
    int red = map(analogRead(LED_CONTROLLER_3_R), 0, 1023, 0, 255);
    int green = map(analogRead(LED_CONTROLLER_3_G), 0, 1023, 0, 255);
    int blue = map(analogRead(LED_CONTROLLER_3_B), 0, 1023, 0, 255);

    // Set the data bytes
    tx_msg.data[1] = 0xFE;
    tx_msg.data[2] = 1;
    tx_msg.data[5] = red;
    tx_msg.data[6] = green;
    tx_msg.data[7] = blue;

    // Send the CAN message
    can.write(tx_msg);

}


// Turn off all LEDs
void turn_off_all_leds() {
    // Turn off all LEDs
    digitalWrite(LED_CONTROLLER_1_R, LOW);
    digitalWrite(LED_CONTROLLER_1_G, LOW);
    digitalWrite(LED_CONTROLLER_1_B, LOW);
    digitalWrite(LED_CONTROLLER_2_R, LOW);
    digitalWrite(LED_CONTROLLER_2_G, LOW);
    digitalWrite(LED_CONTROLLER_2_B, LOW);
    digitalWrite(LED_CONTROLLER_3_R, LOW);
    digitalWrite(LED_CONTROLLER_3_G, LOW);
    digitalWrite(LED_CONTROLLER_3_B, LOW);

}

// RGB light show
void rgb_light_show() {
    // Triple for loop to cycle through all color combinations
    for (int r = 0; r < 256; r++) {
        for (int g = 0; g < 256; g++) {
            for (int b = 0; b < 256; b++) {
                // Set RGB values
                set_static_states(1, r, g, b);
                set_static_states(2, r, g, b);
                set_static_states(3, r, g, b);

                // Wait .1 second
                delay(100);

            }
        }
    }
}

// Method to set static states
void set_static_states(int controller, byte r, byte g, byte b) {
    // Set Static States
    switch (controller) {
        case 1:
            digitalWrite(LED_CONTROLLER_1_R, r);
            digitalWrite(LED_CONTROLLER_1_G, g);
            digitalWrite(LED_CONTROLLER_1_B, b);
            break;
        case 2:
            digitalWrite(LED_CONTROLLER_2_R, r);
            digitalWrite(LED_CONTROLLER_2_G, g);
            digitalWrite(LED_CONTROLLER_2_B, b);
            break;
        case 3:
            digitalWrite(LED_CONTROLLER_3_R, r);
            digitalWrite(LED_CONTROLLER_3_G, g);
            digitalWrite(LED_CONTROLLER_3_B, b);
            break;
        default:
            break;

    }
}

// Function to blink each controller n times
void blink(int n) {
    // Blink the LED controller n times
    for (int i = 0; i < n; i++) {
        // Set the LED controller to on
        set_led_controller(1, HIGH);
        set_led_controller(2, HIGH);
        set_led_controller(3, HIGH);

        // Wait .1 second
        delay(100);

        // Set the LED controller to off
        set_led_controller(1, LOW);
        set_led_controller(2, LOW);
        set_led_controller(3, LOW);

    }
}

// Function to blink x led controller n times
void blink(int controller, int n) {
    // Blink the LED controller n times
    for (int i = 0; i < n; i++) {
        // Set the LED controller to on
        set_led_controller(controller, HIGH);

        // Wait .1 second
        delay(100);

        // Set the LED controller to off
        set_led_controller(controller, LOW);

    }
}

// Function to set the led controller on or off
void set_led_controller(int controller, int state) {
    // Set the LED controller to on
    switch (controller) {
        case 1:
            digitalWrite(LED_CONTROLLER_1_R, state);
            digitalWrite(LED_CONTROLLER_1_G, state);
            digitalWrite(LED_CONTROLLER_1_B, state);
            break;
        case 2:
            digitalWrite(LED_CONTROLLER_2_R, state);
            digitalWrite(LED_CONTROLLER_2_G, state);
            digitalWrite(LED_CONTROLLER_2_B, state);
            break;
        case 3:
            digitalWrite(LED_CONTROLLER_3_R, state);
            digitalWrite(LED_CONTROLLER_3_G, state);
            digitalWrite(LED_CONTROLLER_3_B, state);
            break;
        default:
            break;

    }
}