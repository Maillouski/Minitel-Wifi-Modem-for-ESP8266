void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    //Serial.println("In webSocketEvent function");
    switch(type) {
        case WStype_DISCONNECTED:
            yield();
            if(!firstconnect){
              firstconnect = true;
            }
            //Serial.println("WStype_DISCONNECTED");
            //delay(2000);
            webSocket->disconnect();
            webSocket = nullptr;
            Serial.read(); //On vide le buffer
            websocketmode = false;
            break;
        case WStype_CONNECTED:
            yield();
            //Serial.println("Wstype_CONNECTED");
            sendResult(R_CONNECT);
            connectTime = millis();
            //Serial.print("FirstConnect: ");Serial.println(firstconnect);
            if (firstconnect) {
                //sendConnexionFinSequence();
                //sendDeconnexion();
                firstconnect = false;
            } 
            break;
        case WStype_TEXT:
            //Serial.println("WStype_TEXT");delay(1000);
            //Serial.print((char*)payload);
            yield();
              if (!cmdMode){ // If we are in cmdMode, we do not process anything
                led_on();
                for (size_t i = 0; i < length; i++) {
                  yield();
                  uint8_t rxByte = payload[i];

                  // Est-ce que c'est un code de contrôle Telnet qui commence ?
                  if ((telnet == true) && (rxByte == 0xff)) {
                    handleTelnetControlCode(rxByte);
                  } else {
                    // Les codes non-contrôle passent librement
                    Serial.write(rxByte);
                    Serial.flush();
                  }
                }
              }
            break;

        case WStype_BIN:
            yield();
            /*
            Serial.println("[WebSocket] Received binary data:");
            // Pour les données binaires, on peut les afficher en hexadécimal ou les traiter autrement
            for(size_t i = 0; i < length; i++) {
                Serial.printf("%02x ", payload[i]);
            }
            Serial.println();
            */
            break;
        case WStype_ERROR:
            yield();
            Serial.printf("[WebSocket] Error!\n");
            break;
    }
}