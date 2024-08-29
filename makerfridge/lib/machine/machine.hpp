#ifndef MACHINE_HPP
#define MACHINE_HPP

#include "board_framework.hpp"

const int TOTAL_PRODUCTS = 5;
const int MAX_PRODUCT_STOCK = 300;

typedef struct ProductPins
{
    int button = 0;
    int actuator = 0;
} product_pins_t;

typedef struct ProductStats
{
    int current_stock = 0;

} product_stats_t;

typedef struct Product
{
    int previous_button_state = HIGH;
    product_pins_t pins;
    product_stats_t stats;
    bool is_set_for_delivery = false;
} product_t;


typedef struct Machine
{
    Machine(const BoardFramework* boardfw);
    void read_buttons();
    void set_product_stats(const product_stats_t newStats[], unsigned int length);
    bool set_product_stats_from_json(const char* json);
    int deliver_product();
    void blink_out_of_stock_led();
    bool has_products_to_deliver() const;
    bool to_json(char* json_buffer, unsigned int buflen) const;

    product_t machine_products[TOTAL_PRODUCTS];
    const BoardFramework *board;
    const int out_of_stock_led;
} machine_t;

#endif // MACHINE_HPP
