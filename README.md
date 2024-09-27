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

<pre>
esp_fastapiserver.py   <-----------> esp8266ollama
                       <-----------> esp32ollama
                       <-----------> mkr1000ollama
</pre>


# Test by CURL

curl -X POST http://127.0.0.1:5000/generate -H "Content-Type: application/json" -d '{"prompt": "Why is the sky blue?"}'

# Test Prompts

| Prompt Category     | Test Prompts                       | Prompt ID |
|---------------------|------------------------------------|-----------|
| **Factual Question** | What is the capital of Italy?      | 1         |
|                     | Who discovered gravity?            | 2         |
|                     | In which year did World War II end?| 3         |
| **Binary Response**  | Is the Earth round?                | 4         |
|                     | Can penguins fly?                  | 5         |
|                     | Is water liquid?                   | 6         |
| **Simple Arithmetic**| What is 9 plus 1?                  | 7         |
|                     | What is half of 16?                | 8         |
|                     | What is 7 minus 4?                 | 9         |
| **Definition**       | Define a cell.                    | 10        |
|                     | Define a poem.                    | 11        |
|                     | Define a computer.                | 12        |
| **Sentence Completion** | The Sun rises in the...         | 13        |
|                     | Necessity is the mother of…        | 14        |
|                     | Practice makes...                 | 15        |
| **Instruction**      | How to cook rice?                 | 16        |
|                     | How to write a story?             | 17        |
|                     | How to draw a picture?            | 18        |

# Conduct tests using all 18 prompts in sequential manner by changing the BAUD RATE (9600, 57600, 115200) of the Module. 

Record the results into one excel sheet. 

# Below are Test Codes for this study

* esp8266ollama_test.ino
* esp32ollama_test.ino
* mkr1000ollama_test.ino

# Change the LLM (qwen2:0.5b) for each baud rate for above.


