''' para que esto corra en cualquier compu en la terminal hay que tirar esta línea: 
    pip install fastapi uvicorn paho-mqtt

    Después hay que correr el siguiente comando en la misma carpeta donde está este main:
    uvicorn main:app --reload

    http://127.0.0.1:8000/api/ultima_medicion acá va a estar el servidor local y vamos a poder ver la última medición

Para después correr la API hay que hacer esto: 
    1. Dejá la terminal de Python (uvicorn) corriendo como está. No la cierres.
    2. Abrí una segunda ventana de terminal y navegá hasta la misma carpeta donde tenés tu index.html.
    3. Escribí este comando nativo de Python para levantar un servidor web básico:
        Bash
        python -m http.server 8080
    4. Ahora abrí tu navegador y entrá a http://localhost:8080.
'''

import sqlite3
import threading
import paho.mqtt.client as mqtt
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

# --- 1. CONFIGURACIÓN DE LA BASE DE DATOS (SQLite) ---
DB_NAME = "sensores.db"

def init_db():
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    # Creamos la tabla si no existe
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS mediciones (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            dispositivo INTEGER,
            timestamp INTEGER,
            temperatura REAL,
            humedad REAL,
            luz REAL,
            mq2 REAL,
            mq9 REAL,
            sensor_extra INTEGER,
            alarma INTEGER
        )
    ''')
    conn.commit()
    conn.close()

init_db()

# --- 2. CONFIGURACIÓN MQTT ---
MQTT_BROKER = "192.168.5.123" # La IP que tenés en tu código C++
MQTT_PORT = 1884
MQTT_TOPIC = "/esp32/datos_sensores"
MQTT_USER = "esp32"
MQTT_PASS = "mirko15"

def on_connect(client, userdata, flags, rc):
    print(f"Conectado al broker MQTT con código de resultado: {rc}")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    payload = msg.payload.decode('utf-8')
    print(f"Mensaje recibido: {payload}")
    
    # Tu formato en C++ es: "%u&%ld&%.2f&%.2f&%.2f&%.2f&%.2f&%u&%u"
    try:
        datos = payload.split('&')
        if len(datos) == 9:
            conn = sqlite3.connect(DB_NAME)
            cursor = conn.cursor()
            cursor.execute('''
                INSERT INTO mediciones (dispositivo, timestamp, temperatura, humedad, luz, mq2, mq9, sensor_extra, alarma)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (int(datos[0]), int(datos[1]), float(datos[2]), float(datos[3]), float(datos[4]), float(datos[5]), float(datos[6]), int(datos[7]), int(datos[8])))
            conn.commit()
            conn.close()
            print("Dato guardado en SQLite exitosamente.")
    except Exception as e:
        print(f"Error procesando mensaje: {e}")

def start_mqtt():
    client = mqtt.Client()
    client.username_pw_set(MQTT_USER, MQTT_PASS)
    client.on_connect = on_connect
    client.on_message = on_message
    
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_forever()
    except Exception as e:
        print(f"No se pudo conectar al broker MQTT: {e}")

# Corremos el cliente MQTT en un hilo (thread) separado para que no bloquee el servidor web
mqtt_thread = threading.Thread(target=start_mqtt, daemon=True)
mqtt_thread.start()

# --- 3. CONFIGURACIÓN DEL SERVIDOR WEB (FastAPI) ---
app = FastAPI()

# Permitir CORS (para que tu frontend en HTML/JS pueda pedirle datos a esta API sin problemas de seguridad)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/")
def read_root():
    return {"mensaje": "API de Sensores Agrícolas funcionando correctamente"}

@app.get("/api/ultima_medicion")
def obtener_ultima_medicion():
    # Buscamos el último registro en la base de datos
    conn = sqlite3.connect(DB_NAME)
    # Hacemos que devuelva un diccionario en lugar de una tupla
    conn.row_factory = sqlite3.Row 
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM mediciones ORDER BY id DESC LIMIT 1')
    row = cursor.fetchone()
    conn.close()
    
    if row:
        return dict(row)
    return {"error": "No hay datos disponibles todavía"}