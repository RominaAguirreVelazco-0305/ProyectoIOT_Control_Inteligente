#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

const char* ap_ssid = "ESP8266_Access_Point";
const char* ap_password = "12345678";
const char* wifi_ssid = "LOS_CHINGONES_DE_CHINGONES";
const char* wifi_password = "Canisrominaagvela0305";
const char* serverUrl = "http://192.168.100.84:5030/get-answer";
const int ledPin = 2;  // Asegúrate de que el pin del LED es correcto

ESP8266WebServer server(80);


const char* htmlPage = R"=====(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Control Inteligente de LED ESP8266</title>
    <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500;700&display=swap" rel="stylesheet">
    <style>
        :root {
            --primary-color: #ff1493;
            --secondary-color: #8a2be2;
            --background-color: #000000;
            --panel-bg: #1a1a1a;
            --text-color: #ffffff;
            --button-active: #8a2be2;
            --button-inactive: #333333;
            --chat-user: #ff00ff80;  /* Magenta fosforescente más suave con transparencia */
            --chat-bot: #00ffff80;   /* Cian fosforescente más suave con transparencia */
        }
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }
        body {
            font-family: 'Roboto', sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            width: 100%;
            max-width: 800px;
            background-color: var(--panel-bg);
            border-radius: 20px;
            box-shadow: 0 10px 30px rgba(255,20,147,0.3);
            overflow: hidden;
        }
        .header {
            background: linear-gradient(45deg, var(--primary-color), var(--secondary-color));
            color: white;
            padding: 20px;
            text-align: center;
            position: relative;
            overflow: hidden;
        }
        .header::before {
            content: '';
            position: absolute;
            top: -50%;
            left: -50%;
            width: 200%;
            height: 200%;
            background: radial-gradient(circle, rgba(255,255,255,0.3) 0%, rgba(255,255,255,0) 70%);
            animation: ripple 15s infinite;
        }
        @keyframes ripple {
            0% { transform: translate(0, 0); }
            50% { transform: translate(-30%, -30%); }
            100% { transform: translate(0, 0); }
        }
        h1 {
            margin: 0;
            font-size: 24px;
            font-weight: 700;
            position: relative;
            z-index: 1;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .welcome-title {
            font-size: 16px;
            margin-top: 10px;
            opacity: 0.9;
            position: relative;
            z-index: 1;
        }
        .control-panel {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 15px;
            padding: 20px;
            background-color: var(--panel-bg);
        }
        .control-button {
            background-color: var(--button-inactive);
            color: var(--text-color);
            border: 2px solid var(--primary-color);
            padding: 15px;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.3s ease;
            display: flex;
            align-items: center;
            justify-content: center;
            flex-direction: column;
        }
        .control-button:hover {
            background-color: var(--secondary-color);
            transform: translateY(-3px);
            box-shadow: 0 6px 8px rgba(255,20,147,0.3);
        }
        .control-button:active {
            transform: translateY(-1px);
            box-shadow: 0 2px 4px rgba(255,20,147,0.3);
        }
        .control-button .icon {
            font-size: 24px;
            margin-bottom: 8px;
        }
        .control-button.selected {
            background-color: var(--button-active);
            color: white;
        }
        .led-indicator {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background-color: #ccc;
            margin-top: 8px;
            transition: background-color 0.3s ease;
        }
        .led-indicator.on {
            background-color: #4CAF50;
        }
        #chat {
            padding: 20px;
            background-color: var(--panel-bg);
        }
        #messages {
            height: 200px;
            overflow-y: auto;
            margin-bottom: 15px;
            padding: 10px;
            border: 1px solid var(--primary-color);
            border-radius: 10px;
            background: var(--button-inactive);
        }
        #messages::-webkit-scrollbar {
            width: 6px;
        }
        #messages::-webkit-scrollbar-thumb {
            background-color: var(--secondary-color);
            border-radius: 3px;
        }
        #messages::-webkit-scrollbar-track {
            background-color: var(--button-inactive);
            border-radius: 3px;
        }
        #messages div {
            margin-bottom: 10px;
            padding: 8px 12px;
            border-radius: 15px;
            max-width: 80%;
            word-wrap: break-word;
            animation: fadeIn 0.5s;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            font-size: 14px;
        }
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(10px); }
            to { opacity: 1; transform: translateY(0); }
        }
        #messages div:nth-child(odd) {
            background-color: var(--chat-user);
            color: white;
            align-self: flex-end;
            margin-left: auto;
            border-bottom-right-radius: 0;
        }
        #messages div:nth-child(even) {
            background-color: var(--chat-bot);
            color: white;
            border-bottom-left-radius: 0;
        }
        .input-container {
            display: flex;
            gap: 10px;
        }
        #user-input {
            flex-grow: 1;
            padding: 12px;
            border: 2px solid var(--primary-color);
            border-radius: 20px;
            font-size: 14px;
            background-color: var(--button-inactive);
            color: var(--text-color);
            transition: all 0.3s ease;
        }
        #user-input:focus {
            outline: none;
            box-shadow: 0 0 0 3px rgba(255,20,147,0.3);
            transform: translateY(-2px);
        }
        #send-button {
            padding: 12px 20px;
            background: linear-gradient(45deg, var(--primary-color), var(--secondary-color));
            color: white;
            border: none;
            border-radius: 20px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: 0 4px 6px rgba(255,20,147,0.3);
        }
        #send-button:hover {
            background: linear-gradient(45deg, var(--secondary-color), var(--primary-color));
            transform: translateY(-2px);
            box-shadow: 0 6px 8px rgba(255,20,147,0.3);
        }
        #send-button:active {
            transform: translateY(0);
            box-shadow: 0 2px 4px rgba(255,20,147,0.3);
        }
        @media (max-width: 600px) {
            .control-panel {
                grid-template-columns: 1fr;
            }
            .input-container {
                flex-direction: column;
            }
            #send-button {
                width: 100%;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🎮 Control Inteligente de LED ESP8266</h1>
            <div class="welcome-title">🤖 Asistente Virtual ChatGPT</div>
        </div>
        <div class="control-panel">
            <button class="control-button" onclick="setCommand('Enciende la luz')">
                <span class="icon">💡</span>
                Enciende la luz
                <div class="led-indicator"></div>
            </button>
            <button class="control-button" onclick="setCommand('Apaga la luz')">
                <span class="icon">🌑</span>
                Apaga la luz
                <div class="led-indicator"></div>
            </button>
            <button class="control-button" onclick="setCommand('Haz que parpadee la luz')">
                <span class="icon">✨</span>
                Haz que parpadee la luz
                <div class="led-indicator"></div>
            </button>
            <button class="control-button" onclick="setCommand('Apaga y prende la luz')">
                <span class="icon">🔄</span>
                Apaga y prende la luz
                <div class="led-indicator"></div>
            </button>
        </div>
        <div id="chat">
            <div id="messages"></div>
            <div class="input-container">
                <input type="text" id="user-input" placeholder="Escribe tu comando aquí o selecciona una opción arriba...">
                <button id="send-button" onclick="sendMessage()">Enviar</button>
            </div>
        </div>
    </div>
    <script>
        function setCommand(command) {
            document.getElementById('user-input').value = command;
            sendMessage();
        }

        function sendMessage() {
            var input = document.getElementById('user-input');
            var messages = document.getElementById('messages');
            var message = input.value;
            if (message.trim() === '') return;
            
            input.value = ''; // Clear input field
            var userDiv = document.createElement('div');
            userDiv.textContent = '👤 Tú: ' + message;
            messages.appendChild(userDiv);
            messages.scrollTop = messages.scrollHeight;

            // Simulate LED state change
            updateLEDState(message);

            fetch('/command', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({command: message})
            }).then(function(response) {
                return response.json();
            }).then(function(data) {
                var botDiv = document.createElement('div');
                botDiv.textContent = '🤖 ChatGPT: ' + addEmoji(data.response);
                messages.appendChild(botDiv);
                messages.scrollTop = messages.scrollHeight;
            }).catch(function(error) {
                console.error('Error:', error);
                var errorDiv = document.createElement('div');
                errorDiv.textContent = '❌ Error: No se pudo conectar con el ESP8266';
                messages.appendChild(errorDiv);
                messages.scrollTop = messages.scrollHeight;
            });
        }

        function updateLEDState(command) {
            const buttons = document.querySelectorAll('.control-button');
            buttons.forEach(button => {
                const indicator = button.querySelector('.led-indicator');
                if (button.textContent.toLowerCase().includes(command.toLowerCase())) {
                    indicator.classList.add('on');
                    button.classList.add('selected');
                } else {
                    indicator.classList.remove('on');
                    button.classList.remove('selected');
                }
            });
        }

        function addEmoji(response) {
            if (response.toLowerCase().includes('encendido')) return '✅ ' + response;
            if (response.toLowerCase().includes('apagado')) return '❎ ' + response;
            if (response.toLowerCase().includes('parpadeo')) return '⚡ ' + response;
            return '🔧 ' + response;
        }

        // Permite enviar el mensaje con la tecla Enter
        document.getElementById('user-input').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                sendMessage();
            }
        });
    </script>
</body>
</html>
)=====";


void handleCommand(const String& command, String& responseMessage) {
    if (command.equalsIgnoreCase("Enciende la luz")) {
        digitalWrite(ledPin, LOW);  // Cambiado a LOW para encender
        responseMessage = "LED encendido";
    } else if (command.equalsIgnoreCase("Apaga la luz")) {
        digitalWrite(ledPin, HIGH);  // Cambiado a HIGH para apagar
        responseMessage = "LED apagado";
    } else if (command.equalsIgnoreCase("Haz que parpadee la luz")) {
        for (int i = 0; i < 5; i++) {  // Parpadea el LED 5 veces
            digitalWrite(ledPin, LOW);  // Cambiado a LOW para encender
            delay(200);
            digitalWrite(ledPin, HIGH);  // Cambiado a HIGH para apagar
            delay(200);
        }
        responseMessage = "LED ha parpadeado";
    } else if (command.equalsIgnoreCase("Apaga y prende la luz")) {
        digitalWrite(ledPin, HIGH);  // Cambiado a HIGH para apagar inicialmente
        delay(500);
        digitalWrite(ledPin, LOW);  // Cambiado a LOW para encender
        delay(500);
        digitalWrite(ledPin, HIGH);  // Cambiado a HIGH para apagar nuevamente
        responseMessage = "LED apagado y encendido";
    } else {
        responseMessage = "Comando no reconocido";
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ap_ssid, ap_password);
    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", []() {
        server.send(200, "text/html", htmlPage);
    });

    server.on("/command", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(200);
            deserializeJson(doc, server.arg("plain"));
            String command = doc["command"];
            String responseMessage;

            handleCommand(command, responseMessage);

            DynamicJsonDocument responseDoc(200);
            responseDoc["response"] = responseMessage;
            String jsonResponse;
            serializeJson(responseDoc, jsonResponse);
            server.send(200, "application/json", jsonResponse);
        } else {
            server.send(400, "application/json", "{\"error\":\"No command received\"}");
        }
    });

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}