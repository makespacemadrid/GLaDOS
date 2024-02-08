FROM ghcr.io/coqui-ai/tts

# Establece el directorio de trabajo en /app
WORKDIR /app
# Copia el archivo de requerimientos al contenedor
COPY requirements.txt .


# Instala las dependencias
RUN pip install -r requirements.txt

# Copia el archivo node.py al contenedor
COPY main.py .


# Ejecuta el programa cuando el contenedor se inicie
ENTRYPOINT [""]

CMD ["python3", "/app/main.py"]