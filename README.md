
# 📍 Módulo Inteligente Geolocalizador (Visitas 2026)

### Guía de despliegue de la Web API - Versión 1

Este documento detalla los pasos necesarios para ejecutar la Web API y visualizar los datos del **Módulo Inteligente Geolocalizador**.

-----

## 🛠️ Requisitos Previos

Resulta importante instalar las dependencias necesarias. En la terminal se debe ejeutar el siguiente comando:

```bash
pip install fastapi uvicorn paho-mqtt
```

-----

## 🚀 Paso 1: Levantar el Servidor Backend (FastAPI)

En la carpeta donde se encuentra el archivo `main.py`, inicia el servidor con el siguiente comando:

```bash
    uvicorn main:app --reload
```
Se puede ver la última medición en formato JSON crudo ingresando a:
    > 🔗 [http://127.0.0.1:8000/api/ultima\_medicion](http://127.0.0.1:8000/api/ultima_medicion)

Nota: si se cerró y abrió de nuevo la terminal, simplemente hay que correr de nuevo ambos pasos: tanto el instalador como levantar el servidor. 

-----

## 🌐 Paso 2: Levantar el Servidor Frontend

Para visualizar la interfaz correctamente, **sin cerrar la terminal anterior**:

1.  **Mantené la terminal de Python (`uvicorn`) activa.** 
2.  Abriendo una **segunda ventana de terminal**, navega hasta la carpeta donde se encuentra tu archivo `index.html`.
3.  Inicia un servidor web básico con Python:
    ```bash
    python -m http.server 8080
    ```
    Alternativamente, si se usa python3, 
    ```bash
    python3 -m http.server 8080
    ```
4.  **Acceso final:** Entra a la siguiente dirección para ver la web API:
    > 🔗 [http://localhost:8080](https://www.google.com/search?q=http://localhost:8080)

-----

### 📝 Notas adicionales

  * El servidor backend utiliza **Uvicorn** para manejar las peticiones asíncronas.
  * El frontend se sirve de forma local en el puerto **8080** para evitar conflictos con la API.
  * Cuando se usan datos **simulados** el banner superior figurará en verde.
  * Cuando se usan datos **reales** el banner superior figurará en azul. 


<!-- # MODULO-INTELIGENTE-GEOLOCALIZADOR-VISITAS-2026


PASOS PARA CORRER LA WEB API LEVANTANDO LOS DATOS DEL _MÓDULO INTELIGENTE GEOLOCALIZADOR_ (~~la casita PID~~) en la versión 1: 
Para que esto corra en cualquier compu en la terminal hay que tirar esta línea: 
    <pip install fastapi uvicorn paho-mqtt>

  Después hay que correr el siguiente comando en la misma carpeta donde está este main:
    <uvicorn main:app --reload>

  En http://127.0.0.1:8000/api/ultima_medicion -> va a estar el servidor local y vamos a poder ver la última medición en un JSON bien crudo

Para después correr la API hay que hacer esto: 
    1. Dejá la terminal de Python (<uvicorn>) corriendo como está. No la cierres.
    2. Abrí una segunda ventana de terminal y navegá hasta la misma carpeta donde tenés tu index.html.
    3. Escribí este comando nativo de Python para levantar un servidor web básico:
        <Bash>
        <python -m http.server 8080>
    4. Ahora abrí tu navegador y entrá a http://localhost:8080. -->
