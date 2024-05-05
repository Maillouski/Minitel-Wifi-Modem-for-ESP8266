void handleGopherRequest(){
  // From the URL, aquire required variables
  // (14 = "ATGPHgopher://")
  int portIndex = cmd.indexOf(":", 14); // Index where port number might begin
  int pathIndex = cmd.indexOf("/", 14); // Index first host name and possible port ends and path begins
  int port;
  String path, host;
  if (pathIndex < 0)
  {
    pathIndex = cmd.length();
  }
  if (portIndex < 0)
  {
    port = 70;
    portIndex = pathIndex;
  }
  else
  {
    port = cmd.substring(portIndex + 1, pathIndex).toInt();
  }
  host = cmd.substring(14, portIndex);
  path = cmd.substring(pathIndex, cmd.length());
  if (path == "") path = "/";
  char *hostChr = new char[host.length() + 1];
  host.toCharArray(hostChr, host.length() + 1);

  // Establish connection
  if (!tcpClient.connect(hostChr, port))
  {
    sendResult(R_NOCARRIER);
    connectTime = 0;
    callConnected = false;
    setCarrierDCDPin(callConnected);
  }
  else
  {
    sendResult(R_CONNECT);
    connectTime = millis();
    cmdMode = false;
    callConnected = true;
    setCarrierDCDPin(callConnected);
    tcpClient.print(path + "\r\n");
  }
  delete hostChr;  
}
