#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

ESP8266HTTPUpdateServer httpUpdater;
ESP8266WebServer webServer(80);

#include "webserver.h" //Our HTML webpage contents

void handleWebServer(){
  // Service the Web server
  webServer.handleClient();
}

void webserverSetup(){
  webServer.on("/", handleRoot);
  webServer.on("/ath", handleWebHangUp);
  webServer.on("/reboot", handleReboot);
  webServer.on("/get-status", handleGetStatus);
  webServer.on("/get-settings", handleGetSettings);
  webServer.on("/update-settings", handleUpdateSettings);
  webServer.on("/update-firmware", handleUpdateFirmware);
  webServer.on("/update-speeddial", handleUpdateSpeeddial);
  webServer.on("/factory-defaults", handleFactoryDefaults);
  webServer.on("/file-upload", handleFileUpload);
  webServer.begin();
}

void handleUpdateSettings(){
  return redirectToRoot();  
}
void handleUpdateFirmware(){
  firmwareUpdating = true;
  return redirectToRoot();  
}
void handleUpdateSpeeddial(){
  return redirectToRoot(); 
}
void handleFactoryDefaults(){
  defaultEEPROM();
  readSettings();
  sendResult(R_OK);  
  return redirectToRoot();   
}
void handleFileUpload(){
  return redirectToRoot();   
}
void handleGetSettings(){
  String json = "{ ";
  json += "\"echo\": \"" + String(echo) + "\",";
  json += "\"autoAnswer\": \"" + String(autoAnswer) + "\",";
  json += "\"serialspeed\": \"" + String(serialspeed) + "\",";
  json += "\"ssid\": \"" + String(ssid) + "\",";
  json += "\"busyMsg\": \"" + String(busyMsg) + "\",";
  json += "\"tcpServerPort\": \"" + String(tcpServerPort) + "\",";
  json += "\"telnet\": \"" + String(telnet) + "\",";
  json += "\"verboseResults\": \"" + String(verboseResults) + "\",";
  json += "\"flowControl\": \"" + String(flowControl) + "\",";
  json += "\"pinPolarity\": \"" + String(pinPolarity) + "\",";
  json += "\"quietMode\": \"" + String(quietMode) + "\"";
  json += "}";
  webServer.send(200, "application/json", json);
  
}

void handleGetSpeedDials(){
//  for (int i = 0; i < 10; i++) {
//      speedDials[i];
//  }
}

void handleGetStatus(){
  String json = "{ ";
    json += "\"wifiStatus\": \"" + getWifiStatus() + "\",";
    json += "\"ssidStatus\": \"" + WiFi.SSID() + "\",";
    json += "\"macAddress\": \"" + getMacAddress() + "\",";
    json += "\"ipAddress\": \"" + ipToString(WiFi.localIP()) + "\",";
    json += "\"gateway\": \"" + ipToString(WiFi.gatewayIP()) + "\",";
    json += "\"subnet\": \"" + ipToString(WiFi.subnetMask()) + "\",";
    json += "\"serverPort\": \"" + String(tcpServerPort) + "\",";
    json += "\"callStatus\": \"" + getCallStatus() + "\",";
    json += "\"callLength\": \"" + getCallLength() + "\",";
    json += "\"baudStatus\": \"" + String(bauds[serialspeed]) + "\"";
  json += "}";
  webServer.send(200, "application/json", json);
}

String getWifiStatus(){
  if (WiFi.status() == WL_CONNECTED) {
    return "CONNECTED";
  }
  if (WiFi.status() == WL_IDLE_STATUS) {
    return "OFFLINE";
  }
  if (WiFi.status() == WL_CONNECT_FAILED) {
    return "CONNECT FAILED";
  }
  if (WiFi.status() == WL_NO_SSID_AVAIL) {
    return "SSID UNAVAILABLE";
  }
  if (WiFi.status() == WL_CONNECTION_LOST) {
    return "CONNECTION LOST";
  }
  if (WiFi.status() == WL_DISCONNECTED) {
    return "DISCONNECTED";
  }
  if (WiFi.status() == WL_SCAN_COMPLETED) {
    return "SCAN COMPLETED";
  }  
  return "ERROR";
}

String getMacAddress(){
  byte mac[6];
  WiFi.macAddress(mac);
  String macAddress = "";
  macAddress.concat(String(mac[0], HEX));
  macAddress.concat(":");
  macAddress.concat(String(mac[1], HEX));
  macAddress.concat(":");
  macAddress.concat(String(mac[2], HEX));
  macAddress.concat(":");
  macAddress.concat(String(mac[3], HEX));
  macAddress.concat(":");
  macAddress.concat(String(mac[4], HEX));
  macAddress.concat(":");
  macAddress.concat(String(mac[5], HEX));
  return macAddress;  
}

String getCallStatus(){
  String status = "";
  if (callConnected) {
    status.concat("CONNECTED TO ");
    if (ppp) {
      status.concat("PPP");
    } else {
      status.concat(ipToString(tcpClient.remoteIP()));
    }
  //    yield(); //why?
  } else {
    status.concat("NOT CONNECTED");
  }
  return status;
}

String getCallLength(){
  String status = "";
  if (callConnected) {
    status.concat(connectTimeString()); 
  } else {
    status.concat("00:00:00");
  }
  return status;  
}

void handleWebHangUp() {
  String t = "NO CARRIER (" + connectTimeString() + ")";
  hangUp();
//  webServer.send(200, "text/plain", t);
  redirectToRoot();
}

void handleReboot(){
  Serial.println("Rebooting... (requested from web)");
  redirectToRoot();
  ESP.restart();
}

void handleRoot(){
 String s = MAIN_page; //Read HTML contents
 webServer.send(200, "text/html", s);
 delay(100);
}

void redirectToRoot(){
  webServer.sendHeader("Location", String("/"), true);
  webServer.send ( 302, "text/plain", "");
}
