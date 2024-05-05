#define TX_BUF_SIZE 256    // Buffer where to read from serial before writing to TCP
// (that direction is very blocking by the ESP TCP stack,
// so we can't do one byte a time.)
uint8_t txBuf[TX_BUF_SIZE];

char plusCount = 0;        // Go to AT mode at "+++" sequence, that has to be counted
unsigned long plusTime = 0;// When did we last receive a "+++" sequence

void terminalToTcp() {
    //Serial.println("1");delay(500);
    uint32_t key = getKeyCode(false);
    uint8_t lastKey = 0;
    while (key != 0) {  // Loop to handle consecutive characters from getKeyCode
        yield();
        //Serial.println("2");delay(500);
        // Check if the key is '+'
        if (key == '+') {
            plusCount++;
        } else {
            plusCount = 0; // Reset the counter if the current character is not '+'
        }

        if (plusCount >= 3) {
            plusTime = millis();
        }

        // Check for special key sequence 0x1359 (0x13 followed by 0x59)
        if (lastKey == 0x13 && key == 0x59) {
            //Serial.println("Special key sequence 0x1359 detected!"); // Log or handle the special key sequence
            //Serial.println("2");delay(500);
            hangUp();
        }

        // Update lastKey to the current key for the next loop iteration
        lastKey = key;
        // Prepare data to send over WebSocket
        uint8_t payload[4];
        size_t len = 0;
        for (len = 0; key != 0 && len < 4; len++) {
            yield();
            payload[3-len] = uint8_t(key);
            key = key >> 8;
        }

        // Write the buffer to PPP or TCP finally
        if (ppp) {
            pppos_input(ppp, payload+4-len, len);
        } else {
            //Serial.println("4");delay(500);
            tcpClient.write(payload+4-len, len);
            Serial.flush();
        }
        //Serial.println("4");delay(500);
        Serial.flush();  // Ensure all Serial data is sent before next key read

        // Read next key if available
        key = getKeyCode(false);
    }
}

void terminalToTcp_old(){
    static uint8_t lastByte = 0; // Persist last byte between calls
    bool specialKeySequenceDetected = false; // Flag to detect special sequence

    if (Serial.available()) {
        led_on();

        int max_buf_size;
        if (telnet == true)
            max_buf_size = TX_BUF_SIZE / 2;
        else
            max_buf_size = TX_BUF_SIZE;

        // Read from serial, the amount available up to the maximum size of the buffer
        size_t len = std::min(Serial.available(), max_buf_size);
        Serial.readBytes(txBuf, len);

        // Process each byte for '+++' sequence and special two-byte sequence
        for (int i = 0; i < len; i++) {
            // Check for '+++'
            if (txBuf[i] == '+') {
                plusCount++;
            } else {
                plusCount = 0;
            }
            if (plusCount >= 3) {
                plusTime = millis();
            }

            // Check for special key sequence 0x1359
            if (lastByte == 0x13 && txBuf[i] == 0x59) {
                //hangUp();
            }

            // Update lastByte to the current byte for the next iteration
            lastByte = txBuf[i];
        }

        // Handle telnet specific escaping
        if (telnet == true) {
            for (int i = len - 1; i >= 0; i--) {
                if (txBuf[i] == 0xff) {
                    for (int j = TX_BUF_SIZE - 1; j > i; j--) {
                        txBuf[j] = txBuf[j - 1];
                    }
                    len++;
                }
            }
        }
        
        // Write the buffer to PPP or TCP finally
        if (ppp) {
            pppos_input(ppp, txBuf, len);
        } else {
            tcpClient.write(txBuf, len);
            Serial.flush();
        }
        yield();
    }
}

void terminalToWebSocket() {
    uint32_t key = getKeyCode(false);
    uint8_t lastKey = 0;
    while (key != 0) {  // Loop to handle consecutive characters from getKeyCode
        // Check if the key is '+'
        yield();
        if (key == '+') {
            plusCount++;
        } else {
            plusCount = 0; // Reset the counter if the current character is not '+'
        }
        
        if (plusCount >= 3) {
            plusTime = millis();
        }

        // Check for special key sequence 0x1359 (0x13 followed by 0x59)
        if (lastKey == 0x13 && key == 0x59) {
            //Serial.println("Special key sequence 0x1359 detected!"); // Log or handle the special key sequence
            hangUp();
        }

        // Update lastKey to the current key for the next loop iteration
        lastKey = key;
        // Prepare data to send over WebSocket
        uint8_t payload[4];
        size_t len = 0;
        for (len = 0; key != 0 && len < 4; len++) {
            yield();
            payload[3-len] = uint8_t(key);
            key = key >> 8;
        }

        webSocket->sendTXT(payload+4-len, len);
        Serial.flush();  // Ensure all Serial data is sent before next key read

        // Read next key if available
        key = getKeyCode(false);
    }
}

void handleTelnetControlCode(uint8_t rxByte){
  #ifdef DEBUG
    Serial.print("<t>");
  #endif
  
  rxByte = tcpClient.read();
  if (rxByte == 0xff)
  {
    // 2 times 0xff is just an escaped real 0xff
    Serial.write(0xff); Serial.flush();
  }
  else
  {
    // rxByte has now the first byte of the actual non-escaped control code
    #ifdef DEBUG
      Serial.print(rxByte);
      Serial.print(",");
    #endif
    
    uint8_t cmdByte1 = rxByte;
    rxByte = tcpClient.read();
    uint8_t cmdByte2 = rxByte;
    
    // rxByte has now the second byte of the actual non-escaped control code
    #ifdef DEBUG
      Serial.print(rxByte); Serial.flush();
    #endif
    
    // We are asked to do some option, respond we won't
    if (cmdByte1 == DO)
    {
      tcpClient.write((uint8_t)255); 
      tcpClient.write((uint8_t)WONT); 
      tcpClient.write(cmdByte2);
    }
    // Server wants to do any option, allow it
    else if (cmdByte1 == WILL)
    {
      tcpClient.write((uint8_t)255); 
      tcpClient.write((uint8_t)DO); 
      tcpClient.write(cmdByte2);
    }
  }
  #ifdef DEBUG
    Serial.print("</t>");
  #endif  
}

void tcpToTerminal(){
    //why is txpaused checked here, sounds like the wrong direction.
    //tcp to terminal is receiving
    //shouldn't txpaused be checked in transmitting to pc, so pc can say, slow down?
    while (tcpClient.available() && txPaused == false)
    {
      yield();
      led_on();
      uint8_t rxByte = tcpClient.read();
      // Is a telnet control code starting?
      if ((telnet == true) && (rxByte == 0xff))
      {
        handleTelnetControlCode(rxByte);
      }
      else
      {
        // Non-control codes pass through freely
        Serial.write(rxByte); 
      }
    }
}

void handleEscapeSequence(){
  // If we have received "+++" as last bytes from serial port and there
  // has been over a second without any more bytes
  if (plusCount >= 3)
  {
    if (millis() - plusTime > 1000)
    {
      cmdMode = true;
      sendResult(R_OK);
      plusCount = 0;
    }
  } 
}

void handleConnectedMode(){
  yield();
  terminalToTcp();
  yield();
  tcpToTerminal();
  yield();
  handleEscapeSequence();
  yield();
}

void handleWebSocketConnectedMode(){
  yield();
  terminalToWebSocket();
  yield();
  handleEscapeSequence();
  yield();
}