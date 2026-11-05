#include <math.h>
#include <unity.h>

#include "AuthToken.h"
#include "FanController.h"
#include "SettingsRules.h"
#include "WifiReconnectPolicy.h"

void setUp() {}
void tearDown() {}

namespace {

FanSettings defaultSettings() {
    FanSettings s;
    s.startTemp = 30.0f;
    s.maxTemp = 80.0f;
    s.minPercent = 10;
    s.failsafePercent = 40;
    s.quietMaxPercent = 20;
    return s;
}

// Runs the controller long enough for spin-up and ramping to finish.
uint8_t settle(FanController& fan, uint32_t& now) {
    uint8_t duty = 0;
    for (int i = 0; i < 400; i++) {
        now += 100;
        duty = fan.update(now);
    }
    return duty;
}

const int MIN_DUTY = 26;  // 10 % of 255

}  // namespace

// --- Temperature curve ---

void test_fan_stays_off_at_start_temperature() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 30.0f, now);
    TEST_ASSERT_EQUAL_UINT8(0, settle(fan, now));
}

void test_fan_runs_at_minimum_speed_just_above_start() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 30.1f, now);
    TEST_ASSERT_INT_WITHIN(1, MIN_DUTY, settle(fan, now));
}

void test_fan_runs_at_full_speed_at_max_temperature() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 80.0f, now);
    TEST_ASSERT_EQUAL_UINT8(255, settle(fan, now));
}

void test_fan_speed_is_linear_between_start_and_max() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 55.0f, now);
    TEST_ASSERT_INT_WITHIN(2, (MIN_DUTY + 255) / 2, settle(fan, now));
}

// --- Hysteresis ---

void test_running_fan_keeps_minimum_speed_just_below_start() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 31.0f, now);
    settle(fan, now);
    fan.reportSensor(true, 28.5f, now);
    TEST_ASSERT_INT_WITHIN(1, MIN_DUTY, settle(fan, now));
}

void test_running_fan_stops_once_below_hysteresis_band() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 31.0f, now);
    settle(fan, now);
    fan.reportSensor(true, 27.9f, now);
    TEST_ASSERT_EQUAL_UINT8(0, settle(fan, now));
}

void test_stopped_fan_does_not_restart_inside_hysteresis_band() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 20.0f, now);
    settle(fan, now);
    fan.reportSensor(true, 29.5f, now);
    TEST_ASSERT_EQUAL_UINT8(0, settle(fan, now));
}

// --- Spin-up kick ---

void test_starting_fan_gets_full_speed_kick() {
    FanController fan(defaultSettings());
    fan.reportSensor(true, 31.0f, 1000);
    TEST_ASSERT_EQUAL_UINT8(255, fan.update(1000));
    TEST_ASSERT_EQUAL_UINT8(255, fan.update(1999));
}

void test_kick_ends_at_minimum_speed() {
    FanController fan(defaultSettings());
    fan.reportSensor(true, 31.0f, 1000);
    fan.update(1000);
    TEST_ASSERT_INT_WITHIN(1, MIN_DUTY, fan.update(2000));
}

void test_running_fan_gets_no_second_kick() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 31.0f, now);
    uint8_t settled = settle(fan, now);
    fan.reportSensor(true, 32.0f, now);
    now += 100;
    TEST_ASSERT_INT_WITHIN(5, settled, fan.update(now));
}

// --- Ramping ---

void test_speed_rises_gradually() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 31.0f, now);
    uint8_t settled = settle(fan, now);
    fan.reportSensor(true, 80.0f, now);
    now += 1000;
    uint8_t oneSecondLater = fan.update(now);
    TEST_ASSERT_GREATER_THAN_UINT8(settled, oneSecondLater);
    TEST_ASSERT_LESS_THAN_UINT8(settled + 30, oneSecondLater);
}

void test_speed_falls_gradually_before_switching_off() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 80.0f, now);
    settle(fan, now);
    fan.reportSensor(true, 20.0f, now);
    now += 1000;
    TEST_ASSERT_GREATER_THAN_UINT8(200, fan.update(now));
    TEST_ASSERT_EQUAL_UINT8(0, settle(fan, now));
}

// --- Manual mode ---

void test_manual_speed_ignores_temperature() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 80.0f, now);
    fan.setManual(100);
    TEST_ASSERT_EQUAL_UINT8(100, settle(fan, now));
}

void test_manual_zero_stops_a_hot_fan() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 80.0f, now);
    fan.setManual(0);
    TEST_ASSERT_EQUAL_UINT8(0, settle(fan, now));
}

void test_manual_speed_below_minimum_is_raised_to_minimum() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.setManual(5);
    TEST_ASSERT_INT_WITHIN(1, MIN_DUTY, settle(fan, now));
}

void test_returning_to_auto_follows_temperature_again() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 80.0f, now);
    fan.setManual(100);
    settle(fan, now);
    fan.setAuto();
    TEST_ASSERT_EQUAL_UINT8(255, settle(fan, now));
}

// --- Sensor failure ---

void test_brief_sensor_error_holds_current_speed() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 80.0f, now);
    settle(fan, now);
    fan.reportSensor(false, 0.0f, now);
    now += 10000;
    TEST_ASSERT_EQUAL_UINT8(255, fan.update(now));
    TEST_ASSERT_FALSE(fan.sensorFault(now));
}

void test_lasting_sensor_error_switches_to_failsafe_speed() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 80.0f, now);
    settle(fan, now);
    fan.reportSensor(false, 0.0f, now);
    uint8_t duty = settle(fan, now);
    TEST_ASSERT_TRUE(fan.sensorFault(now));
    TEST_ASSERT_INT_WITHIN(1, percentToDuty(40), duty);
}

void test_missing_sensor_at_boot_reaches_failsafe_speed() {
    FanController fan(defaultSettings());
    uint32_t now = 0;
    fan.reportSensor(false, 0.0f, now);
    TEST_ASSERT_INT_WITHIN(1, percentToDuty(40), settle(fan, now));
}

void test_recovered_sensor_clears_fault() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(false, 0.0f, now);
    settle(fan, now);
    fan.reportSensor(true, 20.0f, now);
    TEST_ASSERT_EQUAL_UINT8(0, settle(fan, now));
    TEST_ASSERT_FALSE(fan.sensorFault(now));
}

void test_manual_speed_is_kept_during_sensor_fault() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.setManual(100);
    fan.reportSensor(false, 0.0f, now);
    TEST_ASSERT_EQUAL_UINT8(100, settle(fan, now));
    TEST_ASSERT_TRUE(fan.sensorFault(now));
    TEST_ASSERT_FALSE(fan.failsafeActive(now));
}

// --- Quiet hours ---

void test_quiet_hours_limit_auto_speed() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.reportSensor(true, 80.0f, now);
    fan.setQuietActive(true);
    TEST_ASSERT_INT_WITHIN(1, percentToDuty(20), settle(fan, now));
}

void test_quiet_hours_do_not_limit_manual_speed() {
    FanController fan(defaultSettings());
    uint32_t now = 1000;
    fan.setManual(200);
    fan.setQuietActive(true);
    TEST_ASSERT_EQUAL_UINT8(200, settle(fan, now));
}

void test_quiet_limit_of_zero_stops_the_fan() {
    FanSettings s = defaultSettings();
    s.quietMaxPercent = 0;
    FanController fan(s);
    uint32_t now = 1000;
    fan.reportSensor(true, 80.0f, now);
    fan.setQuietActive(true);
    TEST_ASSERT_EQUAL_UINT8(0, settle(fan, now));
}

void test_quiet_limit_below_minimum_speed_wins() {
    FanSettings s = defaultSettings();
    s.quietMaxPercent = 5;
    FanController fan(s);
    uint32_t now = 1000;
    fan.reportSensor(true, 80.0f, now);
    fan.setQuietActive(true);
    TEST_ASSERT_INT_WITHIN(1, percentToDuty(5), settle(fan, now));
}

void test_quiet_hours_limit_the_spin_up_kick() {
    FanController fan(defaultSettings());
    fan.reportSensor(true, 80.0f, 1000);
    fan.setQuietActive(true);
    TEST_ASSERT_INT_WITHIN(1, percentToDuty(20), fan.update(1000));
}

void test_quiet_window_inside_and_outside() {
    TEST_ASSERT_TRUE(isInQuietHours(120, 60, 180));
    TEST_ASSERT_TRUE(isInQuietHours(60, 60, 180));
    TEST_ASSERT_FALSE(isInQuietHours(180, 60, 180));
    TEST_ASSERT_FALSE(isInQuietHours(30, 60, 180));
}

void test_quiet_window_crossing_midnight() {
    TEST_ASSERT_TRUE(isInQuietHours(23 * 60, 22 * 60, 6 * 60));
    TEST_ASSERT_TRUE(isInQuietHours(5 * 60, 22 * 60, 6 * 60));
    TEST_ASSERT_FALSE(isInQuietHours(12 * 60, 22 * 60, 6 * 60));
}

void test_quiet_window_with_equal_times_is_never_active() {
    TEST_ASSERT_FALSE(isInQuietHours(100, 100, 100));
}

// --- Settings rules ---

void test_default_temperature_mapping_is_accepted() {
    TEST_ASSERT_TRUE(isValidTempMapping(30.0f, 80.0f));
}

void test_temperature_mapping_limits() {
    TEST_ASSERT_TRUE(isValidTempMapping(0.0f, 120.0f));
    TEST_ASSERT_FALSE(isValidTempMapping(-1.0f, 80.0f));
    TEST_ASSERT_FALSE(isValidTempMapping(30.0f, 121.0f));
    TEST_ASSERT_FALSE(isValidTempMapping(80.0f, 80.0f));
    TEST_ASSERT_FALSE(isValidTempMapping(NAN, 80.0f));
}

void test_blank_mqtt_password_keeps_stored_one() {
    TEST_ASSERT_EQUAL_STRING("secret", resolveMqttPassword("", "user", "secret"));
}

void test_new_mqtt_password_replaces_stored_one() {
    TEST_ASSERT_EQUAL_STRING("new", resolveMqttPassword("new", "user", "secret"));
}

void test_blank_mqtt_user_clears_password() {
    TEST_ASSERT_EQUAL_STRING("", resolveMqttPassword("", "", "secret"));
}

void test_percent_duty_conversion() {
    TEST_ASSERT_EQUAL_UINT8(0, percentToDuty(0));
    TEST_ASSERT_EQUAL_UINT8(128, percentToDuty(50));
    TEST_ASSERT_EQUAL_UINT8(255, percentToDuty(100));
    TEST_ASSERT_EQUAL_UINT8(255, percentToDuty(150));
    TEST_ASSERT_EQUAL_UINT8(50, dutyToPercent(128));
    TEST_ASSERT_EQUAL_UINT8(100, dutyToPercent(255));
}

// --- Session token ---

void test_empty_token_never_matches_empty_session() {
    TEST_ASSERT_FALSE(tokenMatches("", ""));
}

void test_correct_token_matches() {
    TEST_ASSERT_TRUE(tokenMatches("abc123", "abc123"));
}

void test_wrong_token_is_rejected() {
    TEST_ASSERT_FALSE(tokenMatches("abc124", "abc123"));
    TEST_ASSERT_FALSE(tokenMatches("abc12", "abc123"));
    TEST_ASSERT_FALSE(tokenMatches("abc1234", "abc123"));
    TEST_ASSERT_FALSE(tokenMatches("", "abc123"));
}

// --- WiFi reconnect ---

void test_wifi_connected_needs_no_action() {
    WifiReconnectPolicy policy;
    WifiActions a = policy.update(1000, true, false, false);
    TEST_ASSERT_FALSE(a.reconnect);
    TEST_ASSERT_FALSE(a.startAp);
    TEST_ASSERT_FALSE(a.stopAp);
}

void test_wifi_loss_reconnects_immediately_then_waits() {
    WifiReconnectPolicy policy;
    policy.update(1000, true, false, false);
    TEST_ASSERT_TRUE(policy.update(11000, false, false, false).reconnect);
    TEST_ASSERT_FALSE(policy.update(21000, false, false, false).reconnect);
    TEST_ASSERT_TRUE(policy.update(41000, false, false, false).reconnect);
}

void test_wifi_outage_starts_emergency_ap_after_delay() {
    WifiReconnectPolicy policy;
    policy.update(1000, true, false, false);
    TEST_ASSERT_FALSE(policy.update(11000, false, false, false).startAp);
    TEST_ASSERT_TRUE(policy.update(41000, false, false, false).startAp);
}

void test_wifi_outage_never_stops_running_ap() {
    WifiReconnectPolicy policy;
    policy.update(1000, true, true, false);
    WifiActions a = policy.update(41000, false, true, false);
    TEST_ASSERT_FALSE(a.startAp);
    TEST_ASSERT_FALSE(a.stopAp);
}

void test_wifi_recovery_stops_emergency_ap_when_ap_setting_is_off() {
    WifiReconnectPolicy policy;
    policy.update(1000, false, true, false);
    TEST_ASSERT_TRUE(policy.update(11000, true, true, false).stopAp);
}

void test_wifi_recovery_keeps_ap_when_ap_setting_is_on() {
    WifiReconnectPolicy policy;
    policy.update(1000, false, true, true);
    TEST_ASSERT_FALSE(policy.update(11000, true, true, true).stopAp);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_fan_stays_off_at_start_temperature);
    RUN_TEST(test_fan_runs_at_minimum_speed_just_above_start);
    RUN_TEST(test_fan_runs_at_full_speed_at_max_temperature);
    RUN_TEST(test_fan_speed_is_linear_between_start_and_max);
    RUN_TEST(test_running_fan_keeps_minimum_speed_just_below_start);
    RUN_TEST(test_running_fan_stops_once_below_hysteresis_band);
    RUN_TEST(test_stopped_fan_does_not_restart_inside_hysteresis_band);
    RUN_TEST(test_starting_fan_gets_full_speed_kick);
    RUN_TEST(test_kick_ends_at_minimum_speed);
    RUN_TEST(test_running_fan_gets_no_second_kick);
    RUN_TEST(test_speed_rises_gradually);
    RUN_TEST(test_speed_falls_gradually_before_switching_off);
    RUN_TEST(test_manual_speed_ignores_temperature);
    RUN_TEST(test_manual_zero_stops_a_hot_fan);
    RUN_TEST(test_manual_speed_below_minimum_is_raised_to_minimum);
    RUN_TEST(test_returning_to_auto_follows_temperature_again);
    RUN_TEST(test_brief_sensor_error_holds_current_speed);
    RUN_TEST(test_lasting_sensor_error_switches_to_failsafe_speed);
    RUN_TEST(test_missing_sensor_at_boot_reaches_failsafe_speed);
    RUN_TEST(test_recovered_sensor_clears_fault);
    RUN_TEST(test_manual_speed_is_kept_during_sensor_fault);
    RUN_TEST(test_quiet_hours_limit_auto_speed);
    RUN_TEST(test_quiet_hours_do_not_limit_manual_speed);
    RUN_TEST(test_quiet_limit_of_zero_stops_the_fan);
    RUN_TEST(test_quiet_limit_below_minimum_speed_wins);
    RUN_TEST(test_quiet_hours_limit_the_spin_up_kick);
    RUN_TEST(test_quiet_window_inside_and_outside);
    RUN_TEST(test_quiet_window_crossing_midnight);
    RUN_TEST(test_quiet_window_with_equal_times_is_never_active);
    RUN_TEST(test_default_temperature_mapping_is_accepted);
    RUN_TEST(test_temperature_mapping_limits);
    RUN_TEST(test_blank_mqtt_password_keeps_stored_one);
    RUN_TEST(test_new_mqtt_password_replaces_stored_one);
    RUN_TEST(test_blank_mqtt_user_clears_password);
    RUN_TEST(test_percent_duty_conversion);
    RUN_TEST(test_empty_token_never_matches_empty_session);
    RUN_TEST(test_correct_token_matches);
    RUN_TEST(test_wrong_token_is_rejected);
    RUN_TEST(test_wifi_connected_needs_no_action);
    RUN_TEST(test_wifi_loss_reconnects_immediately_then_waits);
    RUN_TEST(test_wifi_outage_starts_emergency_ap_after_delay);
    RUN_TEST(test_wifi_outage_never_stops_running_ap);
    RUN_TEST(test_wifi_recovery_stops_emergency_ap_when_ap_setting_is_off);
    RUN_TEST(test_wifi_recovery_keeps_ap_when_ap_setting_is_on);
    return UNITY_END();
}
