# Utiliza una imagen base de Python
FROM python:3.12

# Establece el directorio de trabajo en /app
WORKDIR /app

# Copia el archivo de requerimientos al contenedor
COPY requirements.txt .

# Copia el archivo node.py al contenedor
COPY gladosMQTT.py .
COPY node.py .

# Instala las dependencias
RUN pip install -r requirements.txt

# Ejecuta el programa cuando el contenedor se inicie
CMD ["python", "node.py"]