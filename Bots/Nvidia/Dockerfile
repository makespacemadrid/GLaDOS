# Base Image with NVIDIA Runtime
FROM nvidia/cuda:12.3.1-runtime-ubuntu22.04
RUN apt-get update && apt-get install -y build-essential cmake python3 python3-pip python-is-python3
ENV LD_LIBRARY_PATH=/usr/local/cuda/lib64:/usr/local/nvidia/lib:/usr/local/nvidia/lib64
# Utiliza una imagen base que ya tiene el cliente mqtt
#FROM makespacemadrid/glados-base-node:1.5


VOLUME [ "/data" ]
# Establece el directorio de trabajo en /app
WORKDIR /app
# Copia el archivo de requerimientos al contenedor
COPY requirements.txt .
RUN pip install -r requirements.txt

# Copia el archivo node.py al contenedor
COPY main.py .
COPY gladosMQTT.py .
USER 1000

# Ejecuta el programa cuando el contenedor se inicie
CMD ["python", "main.py"]
