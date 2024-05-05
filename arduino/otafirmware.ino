#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

ESP8266WiFiMulti WiFiMulti;

String getLatestVersion(){
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, "http://theoldnet.com/ota/latest-version.txt");
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String latestVersion = "unset"; 
  
  if (httpResponseCode>0) {
    latestVersion = http.getString();
    build.trim();
    latestVersion.trim();
  }
  else {
    Serial.println("Issue checking for firmware update");
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
  
  return latestVersion;
}

void check_for_firmware_update(){
  if (build != getLatestVersion()){
      Serial.println("");
      Serial.println("[ FIRMWARE AVAILABLE! ] Update By Typing AT$FW");
  }
}

void update_started() {
  Serial.println("FIRMWARE update process started");
}

void update_finished() {
  firmwareUpdating = false;
  Serial.println("FIRMWARE update process finished");
  Serial.println("Rebooting...");
  Serial.println("");
}

void update_progress(int cur, int total) {
  Serial.printf("DOWNLOADING FIRMWARE: %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("FIRMWARE update fatal error code %d\n", err);
  firmwareUpdating = false;
}

void handleOTAFirmware(){
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    // The line below is optional. It can be used to blink the LED on the board during flashing
    // The LED will be on during download of one buffer of data from the network. The LED will
    // be off during writing that buffer to flash
    // On a good connection the LED should flash regularly. On a bad connection the LED will be
    // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
    // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    // Add optional callback notifiers
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);

    String latestVersion = getLatestVersion();
    
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, "http://theoldnet.com/ota/" + latestVersion + ".bin");
    // Or:
    //t_httpUpdate_return ret = ESPhttpUpdate.update(client, "server", 80, "file.bin");

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        firmwareUpdating = false;
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
  }
}
