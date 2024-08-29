#include "machine.hpp"
#include "board_framework.hpp"
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ArduinoJson.h>

#define ERR_MSG_LEN 100

Machine::Machine(const BoardFramework* boardfw) : board(boardfw), out_of_stock_led(4) {
    // Initialize Machine State
    //
   
    // Black Cable
    machine_products[0].pins.button = 27;
    machine_products[0].pins.actuator = 12;
    
    // Red Cable
    machine_products[1].pins.button = 32;
    machine_products[1].pins.actuator = 14; 
   

    // Orange Cable
    machine_products[2].pins.button = 33;
    machine_products[2].pins.actuator = 19; 
   
    // Yellow Cable
    machine_products[3].pins.button = 25;
    machine_products[3].pins.actuator = 18; 

    // Green Cable
    machine_products[4].pins.button = 26;
    machine_products[4].pins.actuator = 23; 

    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        machine_products[i].previous_button_state = HIGH;
        machine_products[i].stats.current_stock = 0;
        machine_products[i].is_set_for_delivery = false;
        board->pinmode(machine_products[i].pins.button, PIN_MODE::IN_PULLUP);
        board->pinmode(machine_products[i].pins.actuator, PIN_MODE::OUT);
        board->write(machine_products[i].pins.actuator, LOW);
    }
    
    // TODO: The final LED needs a communication protocol for it to work.
    board->pinmode(this->out_of_stock_led, PIN_MODE::OUT);
    board->write(this->out_of_stock_led, HIGH);
}

/**
 * The purpose of this method is to read the state of the buttons.
 *
 * After a button is activated, the product will be set for delivery.
 *
 * If the product is set for delivery, then the button state will not be processed.
 *
 * At each point we can only select one product for delivery, so if we have products to deliver,
 * no more readings will occur.
 *
 * */
void Machine::read_buttons()
{
    char message[ERR_MSG_LEN];
    bzero(&message, ERR_MSG_LEN);

    // Precondition: Only read buttons if there are products to deliver.
    if (this->has_products_to_deliver()) {
        snprintf(message, ERR_MSG_LEN, "Machine has products to deliver, skipping button reading.\n");
                
        board->log(message);
        return;
    }
    
    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        // Buttons in this machine have a resting state of HIGH and a pressed state of LOW.
        int current_button_state = board->read(machine_products[i].pins.button);

        if (current_button_state == LOW) {

            // Confirm button is pressed by testing the value of the button 5 times
            // over a total of 250 ms.
            // This is a necessary operation to avoid false positive when writting code
            // for microcontrollers.
            for (int j=0; j<5; j++) {
                board->fdelay(50);
                if (board->read(machine_products[i].pins.button) == HIGH) {
                    return;
                }
            }

            snprintf(message, ERR_MSG_LEN, "Status for product %d: %d\n", i, current_button_state);
            board->log(message);
            // set product for deliver
            snprintf(message, ERR_MSG_LEN, "Setting product %d for delivery.\n", i);
            board->log(message);
            machine_products[i].is_set_for_delivery = true;
            // Ignore product stock for better UX
//            if (machine_products[i].stats.current_stock > 0) {
//                snprintf(message, ERR_MSG_LEN, "Setting product %d for delivery.\n", i);
//                board->log(message);
//                machine_products[i].is_set_for_delivery = true;
//                return;
//            } else {
//                snprintf(message, ERR_MSG_LEN, "Product %d is out of stock.\n", i);
//                board->log(message);
//                blink_out_of_stock_led();
//                return;
//            }
        } else {
            // Store current button state for next iteration.
            machine_products[i].previous_button_state = current_button_state;
        }
    }
}

void Machine::set_product_stats(const product_stats_t newStats[], unsigned int length)
{
    char message[ERR_MSG_LEN];
    bzero(&message, ERR_MSG_LEN);

    if (length != TOTAL_PRODUCTS) {
        // Verify preconditions
        snprintf(
                message,
                ERR_MSG_LEN,
                "Received incorrect number of stats, expecting %d but got %d.\n",
                TOTAL_PRODUCTS, length);
        board->log(message);
        return;
    }
    for (int i=0; i<TOTAL_PRODUCTS; i++) { 
        machine_products[i].stats.current_stock = newStats[i].current_stock;
    }
    board->log("Statistics for machine products got updated.\n");
}

void Machine::blink_out_of_stock_led() {
    board->log("Product out of stock, blinking out of stock light.\n");
    for (int j=0; j<2; j++) {
        board->write(this->out_of_stock_led, LOW);
        board->fdelay(250);
        board->write(this->out_of_stock_led, HIGH);
        board->fdelay(250);
    }
}

/**
 * Will do the necessary operations for a product marked for delivery.
 *
 * Return -1 if no product has been delivered. 
 * */
int Machine::deliver_product() {
    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        if (machine_products[i].is_set_for_delivery) {

            // Actions uppon delivery.
            //
            // 1. Reset marked for delivery flag
            machine_products[i].is_set_for_delivery = false;
            if (machine_products[i].stats.current_stock > 0) {
                // 2.A Decrement the stock.
                board->log("Decrementing the stock.\n");
                machine_products[i].stats.current_stock -= 1;

            } else {
                // 2.B Blink out of stock light.
                blink_out_of_stock_led();
            }
            // 3. Enable the motor independently of the internal state
            // of the counter for simplifying user experience until the
            // website is ready.
            board->log("Enable the motor.\n");
            board->write(machine_products[i].pins.actuator, HIGH);
            board->fdelay(1000);
            board->log("Disable the motor.\n");
            board->write(machine_products[i].pins.actuator, LOW);
            return i;
        }
    }
    return -1;
}

bool Machine::has_products_to_deliver() const {
    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        if (machine_products[i].is_set_for_delivery) {
            return true;
        }
    }
    return false;
}

/**
 * Fill a character buffer with a JSON-formatted string containing
 * the statistics of the current stock.
 *
 * If there is an overflow in the receiving buffer, then the function
 * will return false.
 * */
bool Machine::to_json(char* json_buffer, unsigned int buflen) const {
    // clean buffer
    bzero(json_buffer, buflen);
    // write to it
    unsigned int written = snprintf(json_buffer, buflen, "{ \"stats\" : { \"p0_stock\" : %d, \"p1_stock\" : %d, \"p2_stock\" : %d, \"p3_stock\" : %d, \"p4_stock\" : %d } }",
            machine_products[0].stats.current_stock,
            machine_products[1].stats.current_stock,
            machine_products[2].stats.current_stock,
            machine_products[3].stats.current_stock,
            machine_products[4].stats.current_stock);

    return written > buflen;
}

bool Machine::set_product_stats_from_json(const char* json) {
    JsonDocument doc;
    product_stats_t newStats[TOTAL_PRODUCTS];
    const char* keys[] = {
        "p0_stock",
        "p1_stock",
        "p2_stock",
        "p3_stock",
        "p4_stock"
    };

    
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        board->log("Error deserializing json.\n");
        board->log(error.c_str());
        board->log("\n");
        return true;
    }

    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        int stock = doc["stats"][keys[i]].as<int>();
        if (stock) {
            if (stock <= 0) {
                char message[ERR_MSG_LEN];
                bzero(&message, ERR_MSG_LEN);
                snprintf(message, ERR_MSG_LEN, "[DESERIALIZATION ERROR] New stock for '%s' has a negative value.\n", keys[i]);
                board->log(message);
                return true;
            }
            if (stock > MAX_PRODUCT_STOCK) {
                // This is to protect from overflows.
                char message[ERR_MSG_LEN];
                bzero(&message, ERR_MSG_LEN);
                snprintf(message, ERR_MSG_LEN, "[DESERIALIZATION ERROR] New stock for '%s' is too large.\n", keys[i]);
                board->log(message);
                return true;
            }
            newStats[i].current_stock = stock;
        } else {
            char message[ERR_MSG_LEN];
            bzero(&message, ERR_MSG_LEN);
            snprintf(message, ERR_MSG_LEN, "[DESERIALIZATION ERROR] Missing '%s' on new stock stats.\n", keys[i]);
            board->log(message);
            return true;
        }
    }
    
    this->set_product_stats(newStats, TOTAL_PRODUCTS);

    return false;
}
