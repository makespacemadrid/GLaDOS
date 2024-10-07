# Utiliza una imagen base de Python
FROM python:3.10

# Establece el directorio de trabajo en /app
WORKDIR /app

# Copia el archivo de requerimientos al contenedor
COPY requirements.txt .
# Instala las dependencias
RUN pip install -r requirements.txt

COPY main.py .

# Ejecuta el programa cuando el contenedor se inicie
CMD ["python", "main.py"]