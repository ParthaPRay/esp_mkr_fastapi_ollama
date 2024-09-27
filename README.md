# esp_mkr_fastapi_ollama
This repo contains the codes about calling FastAPI web server running Ollama for responding back to the prompt served by ESP8266, ESP32 and Arduino MKR 1000

# Keep both Raspberry Pi 4B and generic ESP8266 on same network. It is better to use Mobile Hotspot as Wi-FI mediator for both

1. The fastapisever.py must be run firstly
2. Run the esp8266ollama.ino code next

Server <--------------> Mobile Hotspot <--------------> Esp8266 client

# Run ipconfig (windows), ifconfig(linux) to know about the IP address of Server. Check the  IPv4 Address.

Client sends a prompt to Server (ollama API) on fastapi
Server (ollama API) on fastapi responds json object back to Client

Server <-------------Client (esp8266)
Server -------------> Client (esp8266)

Server <-------------Client (esp32)
Server ------------->Client (esp32)

Server <-------------Client (mkr1000)
Server ------------->Client (mkr1000)

# Run same version codes together as we updated the version of both the systems

esp_fastapiserver.py   <----------->esp8266ollama
                       <----------->esp32ollama
                        <----------->mkr1000ollama

# Test by CURL

curl -X POST http://127.0.0.1:5000/generate -H "Content-Type: application/json" -d '{"prompt": "Why is the sky blue?"}'
