void serialSetup(){

  //  pinMode(DCD_PIN, OUTPUT);
  //Research
  //7 RTS is High from the computer at 9v on other modem
  //8 CTS is high from modem as either output high or input high, not sure
  
  //Consider disabling/ignoring this. PC is going to say RTS high, but do we care since flow control isn't actually implimented in code yet
  //  pinMode(RTS_PIN, INPUT); //this was set to output but the diagrams I look at seem to say a modem's rts pin is input
  //  digitalWrite(RTS_PIN, LOW); // ready to receive data // disabled this because it's input, maybe this is wrong
  
  pinMode(CTS_PIN, OUTPUT); //this was set to input but the diagrams I look at seem to say a modem's cts pin is output
  digitalWrite(CTS_PIN, HIGH); // pull up//old comment
  setCarrierDCDPin(false);
  unsigned long startTime = millis();

  // Check if it's out of bounds-- we have to be able to talk
  if (serialspeed > sizeof(bauds)/sizeof(bauds[0])) {
    serialspeed = 1; //1200 bauds
  }


  // Démarrer le port série USB avec les paramètres lus mode 8N1
  //Serial.begin(bauds[serialspeed], SERIAL_8N1);
  //Serial.println("Press Enter to continue using USB Serial within 5 seconds...");
  //Serial.flush();
/*
  while (millis() - startTime < 5000) {  // Wait for 5 seconds
    if (Serial.available() > 0) {
      yield();
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        Serial.println("USB Serial selected.");
        Serial.flush();
        return;
      }
    }
  }
*/  
  // If no key is pressed, use DE9 Serial // Minitel Mode
  //Serial.print("Switching to DE9 port, Switching Minitel to 4800 and port Speed to 4800 7E1...");
  //Serial.flush();
  Serial.begin(1200, SERIAL_7E1);
  // Clear Serial Buffer
  while (Serial.available()) {
    Serial.read();
  }
  delay(100);
  yield();

  Serial.swap(); // Swap the port to DE9
  delay(100);
  yield();

  setMinitelBaud(minitel4800bauds);
  Serial.flush();
  yield();

  /// MUST END SERIAL BEFORE SWITCH BAUDRATE //
  Serial.end();
  Serial.begin(4800, SERIAL_7E1);
  // Clear Serial Buffer
  while (Serial.available()) {
    Serial.read();
  }
  Serial.swap();
  yield();

  clearMinitelScreen();
  serialspeed = 3; // 4800 bauds
  yield();
  
}

// Fonction pour changer le mode série et le stocker dans l'EEPROM

void setBaudRate(int inSpeed, SerialConfig mode) {
  if (inSpeed == 0) {
    sendResult(R_ERROR);
    return;
  }
  int foundBaud = -1;
  for (unsigned int i = 0; i < sizeof(bauds)/sizeof(bauds[0]); i++) {
    if (inSpeed == bauds[i]) {
      foundBaud = i;
      break;
    }
  }
  // requested baud rate not found, return error
  if (foundBaud == -1) {
    sendResult(R_ERROR);
    return;
  }
  if (foundBaud == serialspeed) {
    sendResult(R_OK);
    return;
  }
  Serial.print("SWITCHING SERIAL PORT TO ");
  Serial.print(inSpeed);
  Serial.println(" IN 3 SECONDS");
  Serial.flush();
  delay(3000);
  Serial.end();
  delay(10);
  Serial.begin(bauds[foundBaud], mode);
  Serial.read();
  //if (DE9 == 1) {Serial.swap();}
  serialspeed = foundBaud;
  sendResult(R_OK);
}
