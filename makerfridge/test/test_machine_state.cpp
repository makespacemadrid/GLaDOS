#include <unity.h>
#include <vector>

#include "machine.hpp"
#include "board_framework_mock.hpp"

BoardFramework* board;
Machine* machineState;

void setUp(void) {
    board = new BoardFrameworkMock();
    machineState = new Machine(board);
}

void tearDown(void) {
    delete machineState;
}

void test_machine_state_initialization() {
    TEST_ASSERT_NOT_NULL(board);
    TEST_ASSERT_NOT_NULL(machineState);
}

void test_machine_stats_setting() {
    product_stats_t stats[TOTAL_PRODUCTS];
    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        stats[i].current_stock = 8;
    }
    machineState->set_product_stats(stats, TOTAL_PRODUCTS);

    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        TEST_ASSERT_EQUAL_INT(8, machineState->machine_products[i].stats.current_stock);
    }
}

void test_machine_stats_to_json() {
    product_stats_t stats[TOTAL_PRODUCTS];
    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        stats[i].current_stock = 8;
    }
    machineState->set_product_stats(stats, TOTAL_PRODUCTS);

    char json[1024];
    bool error = machineState->to_json(json, 1024);

    // Assert there is no error
    TEST_ASSERT_FALSE(error);
    TEST_ASSERT_EQUAL_STRING(
            "{ \"stats\" : { \"p0_stock\" : 8, \"p1_stock\" : 8, \"p2_stock\" : 8, \"p3_stock\" : 8, \"p4_stock\" : 8 } }",
            json);

    // This should return an overflow
    char json2[1];
    bool error2 = machineState->to_json(json2, 1);
    TEST_ASSERT_TRUE(error2);
}

void test_machine_stats_from_json() {
    const char* json = "{ \"stats\" : { \"p0_stock\" : 7, \"p1_stock\" : 7, \"p2_stock\" : 7, \"p3_stock\" : 7, \"p4_stock\" : 7 } }";
    bool error = machineState->set_product_stats_from_json(json);
    TEST_ASSERT_FALSE(error);
    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        TEST_ASSERT_EQUAL_INT(7, machineState->machine_products[i].stats.current_stock);
    }
}

void test_machine_stats_from_json_bad_type() {
    const char* json = "{ \"stats\" : { \"p0_stock\" : \"hello\", \"p1_stock\" : 7, \"p2_stock\" : 7, \"p3_stock\" : 7, \"p4_stock\" : 7 } }";
    bool error = machineState->set_product_stats_from_json(json);
    TEST_ASSERT_TRUE(error);
}

void test_machine_stats_from_json_negative_number() {
    const char* json = "{ \"stats\" : { \"p0_stock\" : -23, \"p1_stock\" : 7, \"p2_stock\" : 7, \"p3_stock\" : 7, \"p4_stock\" : 7 } }";
    bool error = machineState->set_product_stats_from_json(json);
    TEST_ASSERT_TRUE(error);
}

void test_machine_stats_from_json_invalid_attribute_name() {
    const char* json = "{ \"stats\" : { \"asdf\" : 23, \"p1_stock\" : 7, \"p2_stock\" : 7, \"p3_stock\" : 7, \"p4_stock\" : 7 } }";
    bool error = machineState->set_product_stats_from_json(json);
    TEST_ASSERT_TRUE(error);
}

void test_machine_stats_from_json_invalid_number_of_items() {
    const char* json = "{ \"stats\" : { \"p1_stock\" : 7, \"p2_stock\" : 7, \"p3_stock\" : 7, \"p4_stock\" : 7 } }";
    bool error = machineState->set_product_stats_from_json(json);
    TEST_ASSERT_TRUE(error);
}

void test_machine_stats_from_json_stock_overflow() {
    const char* json = "{ \"stats\" : { \"p0_stock\" : 1001, \"p1_stock\" : 7, \"p2_stock\" : 7, \"p3_stock\" : 7, \"p4_stock\" : 7 } }";
    bool error = machineState->set_product_stats_from_json(json);
    TEST_ASSERT_TRUE(error);
}

void test_machine_has_products_to_deliver() {
    // Verify that the machine has no products set to deliver.
    TEST_ASSERT_FALSE(machineState->has_products_to_deliver());

    machineState->machine_products[3].is_set_for_delivery = true;

    // Verify that the machine has products set to deliver.
    TEST_ASSERT_TRUE(machineState->has_products_to_deliver());

    // Deliver the product.
    int delivered_index = machineState->deliver_product();
    TEST_ASSERT_EQUAL_INT(delivered_index, 3);
}

void test_machine_state_read() {
   
    // Put 8 items for each product into the machine .
    //
    product_stats_t stats[TOTAL_PRODUCTS];
    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        stats[i].current_stock = 8;
    }
    machineState->set_product_stats(stats, TOTAL_PRODUCTS);

    // Verify that the machine has no products set to deliver.
    TEST_ASSERT_FALSE(machineState->has_products_to_deliver());

    // Mocker should set button 3 as pressed. 
    machineState->read_buttons();

    // Verify that the machine has products set to deliver.
    TEST_ASSERT_TRUE(machineState->has_products_to_deliver());

    // The product 3 should be set for delivery.
    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        if (i == 3) {
            TEST_ASSERT_TRUE(machineState->machine_products[i].is_set_for_delivery);
        } else {
            TEST_ASSERT_FALSE(machineState->machine_products[i].is_set_for_delivery);
        }
    }

    // When delivering a product, the one delivered should correspond to product 3
    int index_of_delivered = machineState->deliver_product();
    TEST_ASSERT_EQUAL_INT(index_of_delivered, 3);

    // Verify resulting stock.
    // Product 3 should have 7 items while the rest should have 8.
    for (int i=0; i<TOTAL_PRODUCTS; i++) {
        if (i == 3) {
            TEST_ASSERT_EQUAL_INT(7, machineState->machine_products[i].stats.current_stock);
        } else {
            TEST_ASSERT_EQUAL_INT(8, machineState->machine_products[i].stats.current_stock);
        }
    }
}


int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    UNITY_BEGIN();
    RUN_TEST(test_machine_state_initialization);
    RUN_TEST(test_machine_stats_setting);
    RUN_TEST(test_machine_stats_to_json);
    RUN_TEST(test_machine_has_products_to_deliver);
    RUN_TEST(test_machine_state_read);
    RUN_TEST(test_machine_stats_from_json);
    RUN_TEST(test_machine_stats_from_json_bad_type);
    RUN_TEST(test_machine_stats_from_json_negative_number);
    RUN_TEST(test_machine_stats_from_json_invalid_attribute_name);
    RUN_TEST(test_machine_stats_from_json_invalid_number_of_items);
    RUN_TEST(test_machine_stats_from_json_stock_overflow);
    return UNITY_END();
}


