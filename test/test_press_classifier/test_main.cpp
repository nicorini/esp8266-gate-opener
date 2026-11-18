#include <unity.h>

#include "PressClassifier.h"

const unsigned long HOLD_MS = 500;

void setUp() {}
void tearDown() {}

void test_short_press_is_a_click() {
  PressClassifier p(HOLD_MS);
  TEST_ASSERT_EQUAL(PressClassifier::None, p.press(1000, false));
  TEST_ASSERT_EQUAL(PressClassifier::None, p.update(1300));
  TEST_ASSERT_EQUAL(PressClassifier::Click, p.release(1300));
}

void test_hold_starts_at_threshold_once_then_ends_on_release() {
  PressClassifier p(HOLD_MS);
  p.press(1000, false);
  TEST_ASSERT_EQUAL(PressClassifier::None, p.update(1499));
  TEST_ASSERT_EQUAL(PressClassifier::HoldStart, p.update(1500));
  TEST_ASSERT_EQUAL(PressClassifier::None, p.update(1501));
  TEST_ASSERT_EQUAL(PressClassifier::None, p.update(4000));
  TEST_ASSERT_EQUAL(PressClassifier::HoldEnd, p.release(4000));
}

void test_basic_mode_holds_at_press() {
  PressClassifier p(HOLD_MS);
  TEST_ASSERT_EQUAL(PressClassifier::HoldStart, p.press(1000, true));
  TEST_ASSERT_EQUAL(PressClassifier::None, p.update(1500));
  TEST_ASSERT_EQUAL(PressClassifier::HoldEnd, p.release(1100));
}

void test_progress_fills_during_the_threshold() {
  PressClassifier p(HOLD_MS);
  TEST_ASSERT_EQUAL_UINT8(0, p.progress(900));
  p.press(1000, false);
  TEST_ASSERT_EQUAL_UINT8(0, p.progress(1000));
  TEST_ASSERT_EQUAL_UINT8(50, p.progress(1250));
  TEST_ASSERT_EQUAL_UINT8(100, p.progress(1500));
  TEST_ASSERT_EQUAL_UINT8(100, p.progress(3000));
  p.release(3000);
  TEST_ASSERT_EQUAL_UINT8(0, p.progress(3000));
}

void test_survives_millis_overflow() {
  PressClassifier p(HOLD_MS);
  unsigned long start = (unsigned long)-200;
  p.press(start, false);
  TEST_ASSERT_EQUAL(PressClassifier::None, p.update(start + 100));  // before the wrap
  TEST_ASSERT_EQUAL_UINT8(50, p.progress(start + 250));
  TEST_ASSERT_EQUAL(PressClassifier::None, p.update(start + 499));
  TEST_ASSERT_EQUAL(PressClassifier::HoldStart, p.update(start + 500));
  TEST_ASSERT_EQUAL(PressClassifier::HoldEnd, p.release(start + 600));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_short_press_is_a_click);
  RUN_TEST(test_hold_starts_at_threshold_once_then_ends_on_release);
  RUN_TEST(test_basic_mode_holds_at_press);
  RUN_TEST(test_progress_fills_during_the_threshold);
  RUN_TEST(test_survives_millis_overflow);
  return UNITY_END();
}
