#include <unity.h>

#include "TxWindow.h"

const unsigned long MIN_MS = 500;
const unsigned long MAX_MS = 10000;

void setUp() {}
void tearDown() {}

void test_idle_does_not_transmit() {
  TxWindow w(MIN_MS, MAX_MS);
  TEST_ASSERT_FALSE(w.update(0));
}

void test_short_click_lasts_min_time() {
  TxWindow w(MIN_MS, MAX_MS);
  w.press(1000);
  w.release();
  TEST_ASSERT_TRUE(w.update(1499));
  TEST_ASSERT_FALSE(w.update(1500));
}

void test_hold_transmits_until_release() {
  TxWindow w(MIN_MS, MAX_MS);
  w.press(0);
  TEST_ASSERT_TRUE(w.update(3000));
  w.release();
  TEST_ASSERT_FALSE(w.update(3001));
  TEST_ASSERT_FALSE(w.takeTimeout());
}

void test_long_hold_times_out_once() {
  TxWindow w(MIN_MS, MAX_MS);
  w.press(0);
  TEST_ASSERT_TRUE(w.update(9999));
  TEST_ASSERT_FALSE(w.update(10000));
  TEST_ASSERT_TRUE(w.takeTimeout());
  TEST_ASSERT_FALSE(w.takeTimeout());
  TEST_ASSERT_FALSE(w.update(12000));  // still held: no restart until a new press
}

void test_survives_millis_overflow() {
  TxWindow w(MIN_MS, MAX_MS);
  unsigned long start = (unsigned long)-200;
  w.press(start);
  w.release();
  TEST_ASSERT_TRUE(w.update(start + 499));
  TEST_ASSERT_FALSE(w.update(start + 500));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_idle_does_not_transmit);
  RUN_TEST(test_short_click_lasts_min_time);
  RUN_TEST(test_hold_transmits_until_release);
  RUN_TEST(test_long_hold_times_out_once);
  RUN_TEST(test_survives_millis_overflow);
  return UNITY_END();
}
