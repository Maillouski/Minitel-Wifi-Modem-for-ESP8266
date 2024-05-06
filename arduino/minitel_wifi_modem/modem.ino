void webSocketdialStringCleanup(String* host, String* port, String* path, String* dialString, String* wsProtocol)
{
  
  // Extract the protocol by finding the first ':' in the string
  int protocolEndIndex = dialString->indexOf(":");
  if (protocolEndIndex != -1) {
      *wsProtocol = dialString->substring(0, protocolEndIndex);
  } else {
      *wsProtocol = ""; // Default to empty if no ':' found (although unlikely for URLs)
  }
  
  int start = dialString->indexOf("//") + 2; // Start after "//"
  int end = dialString->indexOf("/", start); // Find the first '/' after the "//"
  if (end == -1) {
      end = dialString->length(); // If there's no '/', take the whole string as endpoint
  }

  // Find the port by locating the colon after the host start
  int portIndex = dialString->lastIndexOf(":", end); // Last ':' before the end or path
  if (portIndex > start) {
      *host = dialString->substring(start, portIndex);
      if (end > portIndex) {
          *port = dialString->substring(portIndex + 1, end); // Extract the port correctly
      } else {
          *port = dialString->substring(portIndex + 1); // If no '/', port goes to the end of the string
      }
  } else {
      *host = dialString->substring(start, end); // If no ':' after host, use end
      *port = "80"; // Default WebSocket port if not specified
  }

  // Extract path, starting from '/'
  int pathIndex = dialString->indexOf('?', end);
  if (pathIndex != -1) {
      *path = dialString->substring(end, pathIndex);
  } else if (end != dialString->length()) {
      *path = dialString->substring(end); // Path starts from '/' found after host
  } else {
      *path = "/"; // Default path if none specified
  }
}

String connectTimeString() {
  unsigned long now = millis();
  int secs = (now - connectTime) / 1000;
  int mins = secs / 60;
  int hours = mins / 60;
  String out = "";
  if (hours < 10) out.concat("0");
  out.concat(String(hours));
  out.concat(":");
  if (mins % 60 < 10) out.concat("0");
  out.concat(String(mins % 60));
  out.concat(":");
  if (secs % 60 < 10) out.concat("0");
  out.concat(String(secs % 60));
  return out;
}

void handleQuietMode(String upCmd){
    if (upCmd.substring(3, 4) == "?") {
      sendString(String(quietMode));
      sendResult(R_OK);
    }
    else if (upCmd.substring(3, 4) == "0") {
      quietMode = 0;
      sendResult(R_OK);
    }
    else if (upCmd.substring(3, 4) == "1") {
      quietMode = 1;
      sendResult(R_OK);
    }
    else {
      sendResult(R_ERROR);
    }
}

void telnetDialStringCleanup(String* host, String* port, String* dialString){
  //Serial.println("Telnet Host Detected");
  //Serial.print("dialString: "); Serial.println(*dialString);
  int portIndex = dialString->indexOf(":");
  if (portIndex != -1)
  {
    *host = dialString->substring(0, portIndex);
    *port = dialString->substring(portIndex + 1, dialString->length());
  }
  else
  {
    *host = *dialString;
    *port = "23"; // Telnet default
  }
  //Serial.print("host: "); Serial.println(*host);
  //Serial.print("port: "); Serial.println(*port);
  //Serial.print("path: "); Serial.println(path);  
}

void dialOut(String upCmd) {
  // Can't place a call while in a call
  if (callConnected) {
    sendResult(R_ERROR);
    return;
  }

  String host, port, path, dialString, wsProtocol;
  int portIndex, pathIndex;
  bool hostiswebsocket = false;

  if (upCmd.startsWith("ATDS")) {

    //Serial.println("// Dialing a stored number");
    byte speedNum = upCmd.substring(4, 5).toInt();
    dialString = speedDials[speedNum];
    dialString.trim();
    dialString.toLowerCase();
    //Serial.println(dialString);
    
    if (dialString.startsWith("ws://") || dialString.startsWith("wss://")) {
      // Serial.println("Host is a WebSocket");
      webSocketdialStringCleanup(&host, &port, &path, &dialString, &wsProtocol);
      hostiswebsocket = true;
    } 
    else // Assume it's a telnet connexion
    {
      hostiswebsocket = false;
      telnetDialStringCleanup(&host, &port, &dialString);
    }
  } else {
      //Serial.println("// Dialing an ad-hoc number");
      dialString = upCmd.substring(4);
      dialString.trim();
      dialString.toLowerCase();
      //Serial.print("dialString: "); Serial.println(dialString);

      if (dialString.startsWith("ws://") || dialString.startsWith("wss://")) {
        //Serial.println("Host is a WebSocket");
        webSocketdialStringCleanup(&host, &port, &path, &dialString, &wsProtocol);
        hostiswebsocket = true;
      } 
      else // it's a telnet connection request
      {
        hostiswebsocket = false;
        telnetDialStringCleanup(&host, &port, &dialString);
      }
  }

  host.trim(); // remove leading or trailing spaces
  port.trim(); // remove leading or trailing spaces
  path.trim(); // remove leading or trailing spaces

  if (host.equals("PPP") || host.equals("777")) {
    if (ppp) {
      Serial.println("PPP already active");
      sendResult(R_ERROR);
      return;
    }
    ppp = pppos_create(&ppp_netif, ppp_output_cb, ppp_status_cb, NULL);
    // usepeerdns also means offer our configured DNS servers during negotiation
    ppp_set_usepeerdns(ppp, 1);
    ppp_set_ipcp_dnsaddr(ppp, 0, ip_2_ip4((const ip_addr_t*)WiFi.dnsIP(0)));
    ppp_set_ipcp_dnsaddr(ppp, 1, ip_2_ip4((const ip_addr_t*)WiFi.dnsIP(1)));

#if PPP_AUTH_SUPPORT
    ppp_set_auth(ppp, PPPAUTHTYPE_NONE, "", "");
    ppp_set_auth_required(ppp, 0);
#endif
    ppp_set_ipcp_ouraddr(ppp, ip_2_ip4((const ip_addr_t*)WiFi.localIP()));
    ppp_set_ipcp_hisaddr(ppp, ip_2_ip4((const ip_addr_t*)IPAddress(192,168,240,2)));
    err_t ppp_err;
    ppp_err = ppp_listen(ppp);
    if (ppp_err == PPPERR_NONE) {
      setModeMixte(); 
      sendResult(R_CONNECT);
      connectTime = millis();
      cmdMode = false;
      callConnected = true;
      setCarrierDCDPin(callConnected);
    } else {
      Serial.println("ppp_listen failed\n");
      ppp_status_cb(ppp, ppp_err, NULL);
      ppp_close(ppp, 1);
      sendResult(R_ERROR);
    }    
    return;
  } else if (hostiswebsocket) {
      websocketmode = true;
      cmdMode = false;
      callConnected = true; // Mark the call as connected      
      webSocket = new WebSocketsClient();
      if (wsProtocol == "ws") {
        yield(); 
        Serial.print("DIALING ");Serial.println(host);
        yield();
        webSocket->begin(host.c_str(), port.toInt(), path);
        yield(); 
      }
      if (wsProtocol == "wss") {
        yield(); 
        Serial.print("SSL DIALING ");Serial.println(host);
        yield(); 
        webSocket->beginSSL(host.c_str(), port.toInt(), path.c_str());
        yield(); 
      }
      yield(); 
      webSocket->onEvent(webSocketEvent);
      pageMode(); // put minitel back into page mode
    }
    else
    {
      // TELNET
      Serial.print("DIALING TELNET "); Serial.print(host); Serial.print(":"); Serial.println(port);
      yield();
      delay(1000);
      yield();
      char *hostChr = new char[host.length() + 1];
      host.toCharArray(hostChr, host.length() + 1);
      int portInt = port.toInt();
      tcpClient.setNoDelay(true); // Try to disable naggle
      if (tcpClient.connect(hostChr, portInt))
      {
        //Serial.println("Dans le if de tcpClient.connect");
        tcpClient.setNoDelay(true); // Try to disable naggle
        sendResult(R_CONNECT);
        connectTime = millis();
        cmdMode = false;
        Serial.flush();
        callConnected = true;
        setCarrierDCDPin(callConnected);
        firstconnect = true;
        if(host != "glasstty.com" && host != "bbs.retrocampus.com") // we assume a normal telnet connexion not to a Minitel service.
        {
          setModeMixte();
        }
        else
        {
          pageMode(); // put minitel back into page mode
          sendConnexionFinSequence(); // for the TELSTAR Telnet servers.
        }
      } 
      else
      {
        sendResult(R_NOANSWER);
        callConnected = false;
        websocketmode = false;
        setCarrierDCDPin(callConnected);
        hangUp();
      }
      delete hostChr;
    }
  
}

/**
   Perform a command given in command mode
*/
void command() {
    //Serial.println("In command function");
    cmd.trim();
    if (cmd == "") return;
    Serial.println();
    String upCmd = cmd;
    upCmd.toUpperCase();

    /**** Just AT ****/
    if (upCmd == "AT") sendResult(R_OK);

    /**** Dial to host ****/
    else if ((upCmd.indexOf("ATDT") == 0) || (upCmd.indexOf("ATDP") == 0) || (upCmd.indexOf("ATDI") == 0) || (upCmd.indexOf("ATDS") == 0)) {
        dialOut(upCmd);
    }

    else if (upCmd == "ATTEST"){
        clearMinitelScreen();
        testMode();
        sendResult(R_OK);
    }
    
    /**** Change telnet mode ****/
    else if (upCmd == "ATNET0") {
        telnet = false;
        sendResult(R_OK);
    } else if (upCmd == "ATNET1") {
        telnet = true;
        sendResult(R_OK);
    } else if (upCmd == "ATNET?") {
        Serial.println(String(telnet));
        sendResult(R_OK);
    }

    /**** Answer to incoming connection ****/
    else if ((upCmd == "ATA") && tcpServer.hasClient()) {
        answerCall();
    }

    /**** Display Help ****/
    else if (upCmd == "AT?" || upCmd == "ATHELP") {
        displayHelp();
        sendResult(R_OK);
    }

    /**** Reset, reload settings from EEPROM ****/
    else if (upCmd == "ATZ") {
        readSettings();
        sendResult(R_OK);
    }

    /**** Disconnect WiFi ****/
    else if (upCmd == "ATC0") {
        disconnectWiFi();
        sendResult(R_OK);
    }

    /**** Connect WiFi ****/
    else if (upCmd == "ATC1") {
        connectWiFi();
        sendResult(R_OK);
    }

    /**** Control local echo in command mode ****/
    else if (upCmd.indexOf("ATE") == 0) {
        if (upCmd.substring(3, 4) == "?") {
            sendString(String(echo));
            sendResult(R_OK);
        } else if (upCmd.substring(3, 4) == "0") {
            echo = 0;
            sendResult(R_OK);
        } else if (upCmd.substring(3, 4) == "1") {
            echo = 1;
            sendResult(R_OK);
        } else {
            sendResult(R_ERROR);
        }
    }

    /**** Control verbosity ****/
    else if (upCmd.indexOf("ATV") == 0) {
        if (upCmd.substring(3, 4) == "?") {
            sendString(String(verboseResults));
            sendResult(R_OK);
        } else if (upCmd.substring(3, 4) == "0") {
            verboseResults = 0;
            sendResult(R_OK);
        } else if (upCmd.substring(3, 4) == "1") {
            verboseResults = 1;
            sendResult(R_OK);
        } else {
            sendResult(R_ERROR);
        }
    }

    /**** Control pin polarity of CTS, RTS, DCD ****/
    else if (upCmd.indexOf("AT&P") == 0) {
        if (upCmd.substring(4, 5) == "?") {
            sendString(String(pinPolarity));
            sendResult(R_OK);
        } else if (upCmd.substring(4, 5) == "0") {
            pinPolarity = P_INVERTED;
            sendResult(R_OK);
            setCarrierDCDPin(callConnected);
        } else if (upCmd.substring(4, 5) == "1") {
            pinPolarity = P_NORMAL;
            sendResult(R_OK);
            setCarrierDCDPin(callConnected);
        } else {
            sendResult(R_ERROR);
        }
    }

    /**** Control Flow Control ****/
    else if (upCmd.indexOf("AT&K") == 0) {
        if (upCmd.substring(4, 5) == "?") {
            sendString(String(flowControl));
            sendResult(R_OK);
        } else if (upCmd.substring(4, 5) == "0") {
            flowControl = 0;
            sendResult(R_OK);
        } else if (upCmd.substring(4, 5) == "1") {
            flowControl = 1;
            sendResult(R_OK);
        } else if (upCmd.substring(4, 5) == "2") {
            flowControl = 2;
            sendResult(R_OK);
        } else {
            sendResult(R_ERROR);
        }
    }

    /**** Control Serial Mode ****/
    else if (upCmd.indexOf("AT$SM=") == 0) {
        String modeCmd = upCmd.substring(6);
        if (modeCmd == "7E1") {
            mode = SERIAL_7E1;
            sendResult(R_OK);
            sendString("SWITCHING TO 7E1 IN 5 SECONDS");
            delay(5000);
            Serial.end();
            delay(1000);
            Serial.begin(bauds[serialspeed], mode);
            if (DE9 == 1) {
                Serial.swap();
            }
        } else if (modeCmd == "8N1") {
            mode = SERIAL_8N1;
            sendResult(R_OK);
            sendString("SWITCHING TO 8N1 IN 5 SECONDS");
            delay(5000);
            Serial.end();
            delay(1000);
            Serial.begin(bauds[serialspeed], mode);
            if (DE9 == 1) {
                Serial.swap();
            }
        } else {
            sendResult(R_ERROR);
        }
    } else if (upCmd.indexOf("AT$SM?") == 0) {
        if (mode == SERIAL_7E1) {
            sendString("7E1");
        } else if (mode == SERIAL_8N1) {
            sendString("8N1");
        } else {
            sendString("Unknown mode");
        }
    }

    /**** Set current baud rate ****/
    else if (upCmd.indexOf("AT$SB=") == 0) {
        setBaudRate(upCmd.substring(6).toInt(), mode);
    }

    /**** Display current baud rate ****/
    else if (upCmd.indexOf("AT$SB?") == 0) {
        sendString(String(bauds[serialspeed]));
    }

    /**** Set busy message ****/
    else if (upCmd.indexOf("AT$BM=") == 0) {
        busyMsg = cmd.substring(6);
        sendResult(R_OK);
    }

    /**** Display busy message ****/
    else if (upCmd.indexOf("AT$BM?") == 0) {
        sendString(busyMsg);
        sendResult(R_OK);
    }

    /**** Display Network settings ****/
    else if (upCmd == "ATI") {
        displayNetworkStatus();
        sendResult(R_OK);
    }

    /**** Display profile settings ****/
    else if (upCmd == "AT&V") {
        displayCurrentSettings();
        waitForSpace();
        displayStoredSettings();
        sendResult(R_OK);
    }

    /**** Save (write) current settings to EEPROM ****/
    else if (upCmd == "AT&W") {
        writeSettings();
        sendResult(R_OK);
    }

    else if (upCmd == "AT$FW") {
        firmwareUpdating = true;
    }

    /**** Set or display a speed dial number ****/
    else if (upCmd.indexOf("AT&Z") == 0) {
        byte speedNum = upCmd.substring(4, 5).toInt();
        if (speedNum <= 9) {
            if (upCmd.substring(5, 6) == "=") {
                String speedDial = cmd;
                storeSpeedDial(speedNum, speedDial.substring(6));
                sendResult(R_OK);
            }
            if (upCmd.substring(5, 6) == "?") {
                sendString(speedDials[speedNum]);
                sendResult(R_OK);
            }
        } else {
            sendResult(R_ERROR);
        }
    }

    /**** Set WiFi SSID ****/
    else if (upCmd.indexOf("AT$SSID=") == 0) {
        ssid = cmd.substring(8);
        sendResult(R_OK);
    }

    /**** Display WiFi SSID ****/
    else if (upCmd == "AT$SSID?") {
        sendString(ssid);
        sendResult(R_OK);
    }

    /**** Set WiFi Password ****/
    else if (upCmd.indexOf("AT$PASS=") == 0) {
        password = cmd.substring(8);
        sendResult(R_OK);
    }

    /**** Display WiFi Password ****/
    else if (upCmd == "AT$PASS?") {
        sendString(password);
        sendResult(R_OK);
    }

    /**** Reset EEPROM and current settings to factory defaults ****/
    else if (upCmd == "AT&F") {
        defaultEEPROM();
        readSettings();
        sendResult(R_OK);
    }

    /**** Set auto answer off ****/
    else if (upCmd == "ATS0=0") {
        autoAnswer = false;
        sendResult(R_OK);
    }

    /**** Set auto answer on ****/
    else if (upCmd == "ATS0=1") {
        autoAnswer = true;
        sendResult(R_OK);
    }

    /**** Display auto answer setting ****/
    else if (upCmd == "ATS0?") {
        sendString(String(autoAnswer));
        sendResult(R_OK);
    }

    /**** Set HEX Translate On ****/
    else if (upCmd == "ATHEX=1") {
        hex = true;
        sendResult(R_OK);
    }

    /**** Set HEX Translate Off ****/
    else if (upCmd == "ATHEX=0") {
        hex = false;
        sendResult(R_OK);
    }

    /**** Hang up a call ****/
    else if (upCmd.indexOf("ATH") == 0) {
        hangUp();
    }

    /**** Reboot ESP ****/
    else if (upCmd.indexOf("AT$RB") == 0) {
        sendResult(R_OK);
        Serial.flush();
        delay(500);
        ESP.reset();
    }

    /**** Exit modem command mode, go online ****/
    else if (upCmd == "ATO") {
        if (callConnected == 1) {
            sendResult(R_CONNECT);
            cmdMode = false;
        } else {
            sendResult(R_ERROR);
        }
    }

    /**** Set incoming TCP server port ****/
    else if (upCmd.indexOf("AT$SP=") == 0) {
        tcpServerPort = upCmd.substring(6).toInt();
        sendString("CHANGES REQUIRES NV SAVE (AT&W) AND RESTART");
        sendResult(R_OK);
    }

    /**** Display incoming TCP server port ****/
    else if (upCmd == "AT$SP?") {
        sendString(String(tcpServerPort));
        sendResult(R_OK);
    }

    /**** See my IP address ****/
    else if (upCmd == "ATIP?") {
        Serial.println(WiFi.localIP());
        sendResult(R_OK);
    }

    /**** HTTP GET request ****/
    else if (upCmd.indexOf("ATGET") == 0) {
        handleHTTPRequest();
    }

    /**** Gopher request ****/
    else if (upCmd.indexOf("ATGPH") == 0) {
        handleGopherRequest();
    }

    /**** Control quiet mode ****/
    else if (upCmd.indexOf("ATQ") == 0) {
        handleQuietMode(upCmd);
    }

    /**** Unknown command ****/
    else sendResult(R_ERROR);

    cmd = "";
}

