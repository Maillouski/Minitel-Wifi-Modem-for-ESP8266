const char MAIN_page[] PROGMEM = R"=====(
<html>
    <head>
        <title>The Old Net Serial WIFI Adapter - BUILD 123456</title>
        <!-- <link rel="stylesheet" href="http://10.0.0.238:8080/style.css"> -->
        <style>
            @font-face {
                font-family: dos;
                src: url('http://theoldnet.com/fonts/ModernDOS8x16.ttf')
            }

            body {
                background-color: rgb(111, 198, 213);
                color:rgb(26, 26, 26);
                font-family: dos;
                max-width: 800px;
                margin: auto;
            }

            div {

            }

            .container {
                margin: 6px;
                padding: 6px;
                /* border: 6px double rgb(102, 0, 127); */
                border: 6px double rgb(147, 41, 41);
                background-color: rgb(219, 219, 213);
            }

            ul {
                width: 300px;
            }

            li {
                margin: 2px;
                display: flex;
                flex-direction: row;
                justify-content: space-between;
            }

            .info {
                margin: 6px;
                padding: 6px;
                border: 6px inset rgb(231, 123, 229);
                background-color: rgb(219, 219, 213);
            }

            .speeddial{
                border: none;
            }

            #title-banner {
                display: flex;
                justify-content: space-between;
                align-items: center;
            }
            #modem-icon:hover {
                cursor: alias;
            }
            #firmware-links {
                text-align: right;
            }
        </style>
    </head>
    <body>
        <div id="title-banner" class="container">
            <span>
                <a href="http://theoldnet.com" target="_blank">The Old Net</a> 
                Serial WIFI Adapter
            </span>
            <img id="modem-icon" onclick="toggleModemAudio()" src="http://theoldnet.com/images/dialup3.gif" alt=""></div>
        <div class="container">
            WIFI STATUS: <span id="wifi-status"></span><br>
            SSID.......: <span id="ssid-status"></span></span><br>
            MAC ADDRESS: <span id="mac-address"></span><br>
            IP ADDRESS.: <span id="ip-address"></span><br>
            GATEWAY....: <span id="gateway"></span><br>
            SUBNET MASK: <span id="subnet"></span><br>
            SERVER PORT: <span id="server-port"></span><br>
            BAUD.......: <span id="baud-status"></span><br>
            CALL STATUS: <span id="call-status"></span><br>
            CALL LENGTH: <span id="call-length"></span><br>
            <br>
            <a href="/ath">HANG UP</a>
            <a href="/reboot" onclick="return confirm('Are you sure?')">REBOOT</a>
        </div>
        <div class="container">
            <p>SETTINGS</p>
            <form action="/update-settings" method="GET">
                <ul>
                    <li>
                        <label for="baud">BAUD</label>
                        <select name="baud" id="baud">
                            <option value="0">300</option>
                            <option value="1">1200</option>
                            <option value="2">2400</option>
                            <option value="3">4800</option>
                            <option value="4">9600</option>
                            <option value="5">19200</option>
                            <option value="6">38400</option>
                            <option value="7">57600</option>
                            <option value="8">115200</option>
                        </select>
                    </li>
                    <li>
                        <label for="flow">FLOW CONTROL</label>
                        <select name="flow" id="flow">
                            <option value="0">OFF</option>
                            <option value="1">HARDWARE</option>
                            <option value="2">SOFTWARE</option>
                        </select>            
                    </li>
                    <li>
                        <label for="echo">ECHO</label>
                        <input name="echo" id="echo" type="checkbox">
                    </li>
                    <li>
                        <label for="quiet">QUIET MODE</label>
                        <input name="quiet" id="quiet" type="checkbox">
                    </li>
                    <li>
                        <label for="verbose">VERBOSE MODE</label>
                        <input name="verbose" id="verbose" type="checkbox">
                    </li>
                    <li>
                        <label for="telnet">HANDLE TELNET</label>
                        <input name="telnet" id="telnet" type="checkbox">
                    </li>
                    <li>
                        <label for="polarity">PIN POLARITY</label>
                        <input name="polarity" id="polarity" type="checkbox">
                    </li>
                    <li>
                        <label for="autoanswer">AUTO ANSWER</label>
                        <input name="autoanswer" id="autoanswer" type="checkbox">
                    </li>
                    <li>
                        <label for="busymessage">BUSY MESSAGE</label>
                        <textarea  name="busymessage" id="busymessage"></textarea>
                    </li>
                    <li>
                        <label for="tcpserverport">TCP SERVER PORT</label>
                        <input name="tcpserverport" id="tcpserverport" type="text">
                    </li>
                    <li>
                        <label for="ssid">SSID</label>
                        <input name="ssid" id="ssid" type="text" value="TESTNETWORK">
                    </li>
                    <li>
                        <label for="password">PASSWORD</label>
                        <input name="password" id="password" type="password" value="asdfasdf">
                    </li>
                </ul>
                <input type="submit" value="SAVE">
            </form>
            <div id="firmware-links">
                <a href="/update-firmware" onclick="return confirm('Are you sure?')">UPDATE FIRMWARE</a>
                <a href="/factory-defaults" onclick="return confirm('If you reset factory defaults you will not be able to access this web interface until you configure WIFI again. Are you sure?')">FACTORY DEFAULTS</a>
            </div>
        </div>
        <div class="container">
            <form action="/update-speeddial" method="GET">
                <p>EDIT SPEED DIAL</p>
                <div class="speeddial">
                    <label for="">NAME</label>
                    <input type="text" name="speeddialname1" id="speeddialname1">
                    <label for="">ADDRESS:PORT</label>
                    <input type="text" name="speeddialaddress1" id="speeddialaddress1">
                </div>
                <div class="speeddial">
                    <label for="">NAME</label>
                    <input type="text" name="speeddialname2" id="speeddialname2">
                    <label for="">ADDRESS:PORT</label>
                    <input type="text" name="speeddialaddress2" id="speeddialaddress2">
                </div>
                <input type="submit" value="SAVE">
            </form>
        </div>
        <div class="container">
            <p>Upload File for transfer in terminal</p>
            <form action="/file-upload">
                <input type="file">
                <input type="submit" value="UPLOAD">
            </form>
        </div>        
        <div class="container">
            <div class="info">
                If you do not have a terminal program installed on your vintage computer you can manually type this program into BASIC which will allow you to download a terminal program.
            </div>
            <pre>
                CLS
                PRINT "PC DATA AQUITION"
                PRINT ""
                PRINT "TO EXIT PRESS CTRL+PAUSE"
                INPUT "ENTER FILE NAME: "; N$
                PRINT "TIME        DATA"
                OPEN N$ FOR OUTPUT AS #2
                PRINT #2, "LOGGING STARTED AT "; TIME$; " ON "; DATE$
                PRINT #2, ""
                CLOSE #2
                LOOP1:
                OPEN N$ FOR APPEND AS #2
                OPEN "COM1:9600,N,8,1,CD0,CS0,DS0,OP0,RS,tb2048,rb2048" FOR INPUT AS #1
                a$ = INPUT$(1, 1)
                PRINT TIME$; "     "; a$
                PRINT #2, TIME$; "    "; a$
                CLOSE #1
                CLOSE #2
                GOTO LOOP1
                END
            </pre>
        </div>        
    </body>
    <audio id="modem-audio" src="http://theoldnet.com/audio/dialup.mp3"></audio>
    <script>
        const wifiStatus = document.getElementById('wifi-status')
        const ssidStatus = document.getElementById('ssid-status')
        const macAddress = document.getElementById('mac-address')
        const ipAddress = document.getElementById('ip-address')
        const gateway = document.getElementById('gateway')
        const subnet = document.getElementById('subnet')
        const serverPort = document.getElementById('server-port')
        const callStatus = document.getElementById('call-status')
        const callLength = document.getElementById('call-length')
        const baudStatus = document.getElementById('baud-status')

        const baud = document.getElementById('baud')
        const flow = document.getElementById('flow')
        const echo = document.getElementById('echo')
        const quiet = document.getElementById('quiet')
        const verbose = document.getElementById('verbose')
        const telnet = document.getElementById('telnet')
        const polarity = document.getElementById('polarity')
        const autoanswer = document.getElementById('autoanswer')
        const busymessage = document.getElementById('busymessage')
        const tcpserverport = document.getElementById('tcpserverport')
        const ssid = document.getElementById('ssid')
        const password = document.getElementById('password')

        const modemAudio = document.getElementById('modem-audio')

        const toggleModemAudio = () => {
            modemAudio.paused 
                ? modemAudio.play() 
                : modemAudio.pause();
        }

        const getStatus = () => {
            fetch(`/get-status`)
            .then((response) => response.json())
            .then((data) => {
                console.log(data)
                wifiStatus.innerText = data.wifiStatus
                ssidStatus.innerText = data.ssidStatus
                macAddress.innerText = data.macAddress
                ipAddress.innerText = data.ipAddress
                gateway.innerText = data.gateway
                subnet.innerText = data.subnet
                serverPort.innerText = data.serverPort
                callStatus.innerText = data.callStatus
                callLength.innerText = data.callLength
                baudStatus.innerText = data.baudStatus
            });
        }

        const getSettings = () => {
            fetch(`/get-settings`)
            .then((response) => response.json())
            .then((data) => {
                console.log(data)
                baud.value = data.serialspeed
                ssid.value = data.ssid
                busymessage.value = data.busyMsg
                tcpserverport.value = data.tcpServerPort
                flow.value = data.flowControl
                telnet.checked = Boolean(parseInt(data.telnet))
                verbose.checked = Boolean(parseInt(data.verboseResults))
                polarity.checked = Boolean(parseInt(data.pinPolarity))
                quiet.checked = Boolean(parseInt(data.quietMode))
                echo.checked = Boolean(parseInt(data.echo))
                autoanswer.checked = Boolean(parseInt(data.autoAnswer))
            });
        }        

        getSettings()
        getStatus()
        setInterval(getStatus, 10000)
    </script>
</html>
)=====";
