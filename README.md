# 📘 Proyecto: Control Inteligente de LED con ESP8266 y ChatGPT

### Descripción General 🌟
Este proyecto permite controlar un LED conectado a un ESP8266 mediante comandos enviados desde una página web. Los comandos pueden ser reconocidos y ejecutados gracias a la inteligencia de ChatGPT, que procesa preguntas predefinidas y responde para interactuar con el ESP8266. Puedes encender, apagar o hacer que la luz parpadee con comandos simples y fáciles de entender. Todo esto se realiza mediante un servidor web local que se comunica con el ESP8266 a través de WiFi.

### Funcionalidades Principales 🚀
- 💡 **Encender el LED**: Simplemente escribe "Enciende la luz" y verás el LED encenderse.
- 🌑 **Apagar el LED**: Con el comando "Apaga la luz" puedes apagar el LED.
- ✨ **Hacer que parpadee la luz**: Con "Haz que parpadee la luz", el LED parpadeará varias veces.
- 🔄 **Apagar y prender la luz**: Escribe "Apaga y prende la luz" para alternar su estado.

## 📑 Estructura del Proyecto
El proyecto consta de los siguientes archivos principales:

### 1. Arduino Code (ESP8266) 📟
Este código se carga en el ESP8266 y se encarga de conectarse a la red WiFi, crear un punto de acceso y responder a los comandos HTTP para controlar el LED.

```cpp
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

const char* ap_ssid = "ESP8266_Access_Point";
const char* ap_password = "12345678";
const char* wifi_ssid = "LOS_CHINGONES_DE_CHINGONES";
const char* wifi_password = "Canisrominaagvela0305";
const int ledPin = 2;

ESP8266WebServer server(80);

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
```

### 2. Backend Server (Node.js) 💻
Este archivo es el backend que se comunica con el ESP8266 y ChatGPT. Procesa las peticiones del usuario y envía las respuestas a través de la API de OpenAI.

```javascript
const fs = require('fs');
const express = require('express');
const axios = require('axios');
const cors = require('cors');
const app = express();
const PORT = process.env.PORT || 5030;

// Clave API de OpenAI
const OPENAI_API_KEY = "sk-...";  // Reemplaza con tu clave real de OpenAI

app.use(cors({
    origin: '*',
    methods: ['GET', 'POST', 'OPTIONS'],
    allowedHeaders: ['Content-Type', 'Authorization']
}));

app.use(express.json());

let preguntasRespuestas;
try {
    const data = fs.readFileSync('preguntas.json', 'utf-8');
    preguntasRespuestas = JSON.parse(data);
} catch (err) {
    console.error('Error al leer el archivo preguntas.json:', err);
    preguntasRespuestas = { preguntas: {} };
}

async function sendCommandToESP(command) {
    try {
        const response = await axios.post('http://192.168.100.84/command', { command: command }, {
            headers: {
                'Content-Type': 'application/json'
            }
        });
        return response.data.response;
    } catch (error) {
        console.error("Error al enviar comando al ESP8266:", error);
        return "Error al comunicar con el ESP8266";
    }
}

async function fetchGPTResponse(text) {
    try {
        const response = await axios.post('https://api.openai.com/v1/engines/davinci/completions', {
            prompt: text,
            max_tokens: 150
        }, {
            headers: {
                'Authorization': `Bearer ${OPENAI_API_KEY}`,
                'Content-Type': 'application/json'
            }
        });
        return response.data.choices[0].text.trim();
    } catch (error) {
        console.error('Error calling OpenAI API:', error);
        return "Error al procesar la respuesta de OpenAI";
    }
}

app.get('/', (req, res) => {
    res.json({ status: 'Servidor funcionando correctamente' });
});

app.all('/get-answer', async (req, res) => {
    const pregunta = req.method === 'POST' ? req.body.text : req.query.text;
    if (!pregunta) {
        return res.status(400).json({ response: 'Error: No se recibió ninguna pregunta' });
    }
    let respuesta;
    let comando = "";
    const preguntaKey = pregunta.toLowerCase().trim();
    if (preguntasRespuestas.preguntas.hasOwnProperty(preguntaKey)) {
        respuesta = preguntasRespuestas.preguntas[preguntaKey].respuesta;
        comando = preguntasRespuestas.preguntas[preguntaKey].comando;
        if (comando) {
            const espResponse = await sendCommandToESP(comando);
            respuesta += " " + espResponse;
        }
    } else {
        respuesta = await fetchGPTResponse(pregunta);
    }
    res.json({ response: respuesta });
});

app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});
```

### 3. Archivo de Preguntas (preguntas.json) 📋
Este archivo JSON contiene las respuestas predefinidas para algunas preguntas comunes. ChatGPT se encarga de procesar estas preguntas y responder.

```json
{
  "preguntas": {
    "enciende la luz": {
      "respuesta": "Encendiendo la luz.",
      "comando": "LED_ON"
    },
    "apaga la luz": {
      "respuesta": "Apagando la luz.",
      "comando": "LED_OFF"
    },
    "haz que parpadee la luz": {
      "respuesta": "Haciendo que la luz parpadee.",
      "comando": "LED_TOGGLE"
    },
    "apaga y prende la luz": {
      "respuesta": "Apagando y luego prendiendo la luz.",
      "comando": "LED_TOGGLE"
    }
  }
}
```

## Capturas de Pantalla 📸

### Control Inteligente
![Imagen de WhatsApp 2024-11-11 a las 21 39 49_dadc3475](https://github.com/user-attachments/assets/b58a29f6-f9ee-4411-a0ef-3cf466de768e)
![Imagen de WhatsApp 2024-11-11 a las 21 39 49_3329137e](https://github.com/user-attachments/assets/3681b61a-345d-4ce3-969f-d1b844a81fec)

### Prender la luz
![Imagen de WhatsApp 2024-11-11 a las 21 39 49_6798625f](https://github.com/user-attachments/assets/d62c4b71-a444-48cf-bd0d-cdcd0cb4e73a)
![Imagen de WhatsApp 2024-11-11 a las 18 03 37_64c1fabb](https://github.com/user-attachments/assets/68df37ec-3bb8-44ab-af37-b47563b74292)

### Apagar la luz
![Imagen de WhatsApp 2024-11-11 a las 21 39 50_09d0858a](https://github.com/user-attachments/assets/842fa7fa-af96-4358-b8f6-1153e02565f6)
![Imagen de WhatsApp 2024-11-11 a las 18 03 39_ca6d449c](https://github.com/user-attachments/assets/09db6f36-8c55-425b-a138-6fc3689b281f)

### Luz parpadea
![Imagen de WhatsApp 2024-11-11 a las 21 39 50_e5d42409](https://github.com/user-attachments/assets/634cb205-9716-4336-a117-cae604bd030b)
![Imagen de WhatsApp 2024-11-11 a las 18 03 38_0d50b56d](https://github.com/user-attachments/assets/a885bd9c-a322-4bf5-af3b-50805a020157)

### Luz apaga y prende 
![Imagen de WhatsApp 2024-11-11 a las 21 39 50_de6500ab](https://github.com/user-attachments/assets/47894cb3-751e-4604-92c4-27bd81edf853)
![Imagen de WhatsApp 2024-11-11 a las 18 03 38_658967d8](https://github.com/user-attachments/assets/4d37216c-6e9e-40c0-b4e5-dbff2f7889e5)
![Imagen de WhatsApp 2024-11-11 a las 18 03 37_d4cb16ca](https://github.com/user-attachments/assets/cf04b016-a193-4987-81ad-31f5a7944537)


### Interfaz del Control de LED
Puedes incluir aquí las capturas de pantalla de la interfaz web, mostrando cómo se controla el LED con los diferentes botones y comandos.

## Tecnologías Utilizadas 💻
- **ESP8266**: Para controlar el hardware (LED).
- **HTML/CSS y JavaScript**: Para la interfaz web.
- **Node.js**: Backend para manejar la API y la comunicación con el ESP8266.
- **OpenAI API**: ChatGPT se encarga de procesar las preguntas y respuestas.

## Instrucciones de Uso 📝
1. **Preparar el ESP8266**: Carga el código en el ESP8266 usando el IDE de Arduino.
2. **Configuración del Servidor Backend**: Ejecuta el servidor Node.js con `node server.js`.
3. **Acceder a la Página de Control**: Conéctate a la dirección IP del ESP8266 para acceder a la interfaz de usuario.
4. **Enviar Comandos**: Utiliza la página web para encender, apagar o hacer que el LED parpadee. Puedes escribir los comandos o hacer clic en los botones de la interfaz.

## Instalación y Configuración 🛠️
1. **Clonar el Repositorio**
   ```bash
   git clone <URL DEL REPOSITORIO>
   ```
2. **Instalar Dependencias**
   ```bash
   cd directorio-del-proyecto
   npm install
   ```
3. **Configurar el Backend**
   - Reemplaza la clave de API de OpenAI con la tuya propia.
4. **Ejecutar el Proyecto**
   ```bash
   node server.js
   ```

## Contribuciones ✨
¡Sientéte libre de contribuir al proyecto! Puedes abrir issues, enviar PRs o sugerir mejoras.

## Contacto 📧
Para cualquier consulta, envía un correo a: [tu-email@example.com](mailto:tu-email@example.com).

---

## Desarrollado con ❤️ por Romina Aguirre. 
¡Gracias por interesarte en este proyecto! 🚀


