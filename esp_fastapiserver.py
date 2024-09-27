# This code runs Ollama on FastAPI application based on Uvicorn Web Server
# This code receives API request from ESP8266, generates the text and sends back to ESP8266
# The metrics are logged on local csv file
# Partha Pratim Ray
# 10/9/2024
# This code runs Ollama on FastAPI application based on Uvicorn Web Server
# This code receives API request from ESP8266, generates the text and sends back to ESP8266
# The metrics are logged on local csv file
# Partha Pratim Ray
# 10/9/2024

from fastapi import FastAPI, Request
import requests
import threading
import psutil
import time
import csv
import os
import signal
from pydantic import BaseModel
from queue import Queue  # Importing Queue from the queue module
import uvicorn

app = FastAPI()

# Define the model name as a variable #Change the model as qwen2:0.5b
model_name = "qwen2:0.5b"
OLLAMA_API_URL = "http://localhost:11434/api/generate"

# CSV file setup
csv_file = 'esp_fastapi_api_logs.csv'
csv_headers = [
    'timestamp', 'model_name', 'prompt', 'response', 'eval_count', 'eval_duration',
    'load_duration', 'prompt_eval_duration', 'total_duration', 'tokens_per_second',
    'avg_cpu_usage_during', 'memory_usage_before', 'memory_usage_after',
    'memory_allocated_for_model', 'network_latency', 'total_response_time'
]

csv_queue = Queue()

def csv_writer():
    while True:
        log_message_csv = csv_queue.get()
        if log_message_csv is None:  # Exit signal
            break
        file_exists = os.path.isfile(csv_file)
        with open(csv_file, mode='a', newline='') as file:
            writer = csv.writer(file)
            if not file_exists:
                writer.writerow(csv_headers)
            writer.writerow(log_message_csv)

# Start the CSV writer thread
csv_thread = threading.Thread(target=csv_writer)
csv_thread.start()

class Prompt(BaseModel):
    prompt: str

@app.on_event("startup")
async def startup_event():
    # Measure memory usage for the model
    global memory_allocated_for_model
    memory_allocated_for_model = load_model_and_measure_memory(model_name)
    print(f"Memory Allocated for Model: {memory_allocated_for_model} bytes")

@app.post("/generate")
async def generate(request: Prompt):
    # Log the received request
    print(f"Received a new request with prompt: {request.prompt}")

    start_time = time.time()
    data = request.dict()

    payload = {
        "model": model_name,
        "prompt": data['prompt'],
        "format": "json",
        "stream": False  # Set to True if you want streaming responses
    }

    try:
        # Measure memory usage before the request
        process = psutil.Process()
        memory_usage_before = process.memory_info().rss

        # Start measuring CPU usage in a separate thread
        cpu_usage_result = []
        stop_event = threading.Event()
        cpu_thread = threading.Thread(target=measure_cpu_usage, args=(0.1, stop_event, cpu_usage_result))
        cpu_thread.start()

        # Measure network latency
        network_start_time = time.time()
        # Send the API request
        response = requests.post(OLLAMA_API_URL, json=payload)
        network_latency = (time.time() - network_start_time) * 1e9  # Convert to nanoseconds
        network_latency = round(network_latency, 2)  # Round to 2 decimal points

        # Stop measuring CPU usage
        stop_event.set()
        cpu_thread.join()

        response_data = response.json()

        # Measure memory usage after the request
        memory_usage_after = process.memory_info().rss

        # Calculate average CPU usage during the request
        avg_cpu_usage_during = sum(cpu_usage_result) / len(cpu_usage_result) if cpu_usage_result else 0.0
        avg_cpu_usage_during = round(avg_cpu_usage_during, 2)  # Round to 2 decimal points

        # Extract the required values
        eval_count = response_data.get('eval_count', 0)
        eval_duration = response_data.get('eval_duration', 1)  # avoid division by zero
        load_duration = response_data.get('load_duration', 0)
        prompt_eval_duration = response_data.get('prompt_eval_duration', 0)
        total_duration = response_data.get('total_duration', 0)

        # Calculate tokens per second
        tokens_per_second = eval_count / eval_duration * 1e9 if eval_duration > 0 else 0
        tokens_per_second = round(tokens_per_second, 2)  # Round to 2 decimal points

        # Calculate total response time
        end_time = time.time()
        total_response_time = (end_time - start_time) * 1e9  # Convert to nanoseconds
        total_response_time = round(total_response_time, 2)  # Round to 2 decimal points

        # Log when the prompt is generated and sent back
        print(f"Generated response for prompt: {data['prompt']}, sending it back to the caller.")

        # Prepare log message for CSV
        timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
        log_message_csv = [
            timestamp, model_name, data['prompt'], response_data.get('response', 'N/A'), eval_count,
            eval_duration, load_duration, prompt_eval_duration, total_duration,
            tokens_per_second, avg_cpu_usage_during, memory_usage_before,
            memory_usage_after, memory_allocated_for_model, network_latency, total_response_time
        ]

        # Put the log message into the CSV queue
        csv_queue.put(log_message_csv)

        return response_data
    except requests.exceptions.RequestException as e:
        return {"error": str(e)}

def measure_cpu_usage(interval, stop_event, result):
    while not stop_event.is_set():
        result.append(psutil.cpu_percent(interval=interval))

def load_model_and_measure_memory(model_name):
    # Measure baseline memory usage
    process = psutil.Process()
    baseline_memory = process.memory_info().rss

    # Load the model
    payload = {
        "model": model_name,
        "prompt": "",
        "stream": False  # Just load the model without generating a response
    }
    response = requests.post(OLLAMA_API_URL, json=payload)

    if response.status_code == 200:
        print("Model loaded successfully")
    else:
        print("Failed to load model")

    # Measure memory usage after loading the model
    time.sleep(5)  # Wait for a few seconds to ensure the model is fully loaded
    memory_after_loading = process.memory_info().rss
    memory_allocated_for_model = memory_after_loading - baseline_memory
    return memory_allocated_for_model

# Gracefully handle the shutdown
def handle_shutdown():
    print("Shutting down gracefully...")
    csv_queue.put(None)  # Signal the CSV writer to stop
    csv_thread.join()  # Wait for the CSV writer to finish
    print("Server has been shut down and resources are released.")

def signal_handler(sig, frame):
    handle_shutdown()
    uvicorn_server.should_exit = True  # Set Uvicorn's shutdown flag to True

if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)  # Handle CTRL+C
    uvicorn_server = uvicorn.run(app, host="0.0.0.0", port=5000)
