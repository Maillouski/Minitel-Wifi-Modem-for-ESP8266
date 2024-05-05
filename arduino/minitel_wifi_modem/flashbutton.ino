// Hold for 5 seconds to switch to 300 baud
// Slow flash: keep holding
// Fast flash: let go
int checkButton() {
  long time = millis();
  while (digitalRead(FLASH_BUTTON) == LOW && millis() - time < 5000) {
    //long remaining = millis() - time;
    //Serial.print("\r\n Reset To 300 BPS In " + String(remaining) + "");
    delay(250);
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    digitalWrite(LED_ESP_PIN, !digitalRead(LED_ESP_PIN));
    yield();
  }
  if (millis() - time > 5000) {
    Serial.print("\r\nResetting to 1200 BPS Now");
    Serial.flush();
    Serial.end();
    serialspeed = 1;
    delay(100);
    Serial.begin(bauds[serialspeed]);
    if (DE9 == 1) {Serial.swap();}
    sendResult(R_OK);
    while (digitalRead(FLASH_BUTTON) == LOW) {
      delay(50);
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      digitalWrite(LED_ESP_PIN, !digitalRead(LED_ESP_PIN));
      yield();
    }
    return 1;
  } else {
    return 0;
  }
}
