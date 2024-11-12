const fs = require('fs');
const express = require('express');
const axios = require('axios');
const cors = require('cors');
const app = express();
const PORT = process.env.PORT || 5030;

// Clave API de OpenAI

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
        respuesta = await fetchGPTResponse(pregunta); // Llamada a la API de OpenAI si no hay comando predefinido
    }

    res.json({ response: respuesta });
});

app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});
