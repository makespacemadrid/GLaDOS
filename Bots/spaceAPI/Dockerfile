# Utiliza una imagen base de Python
FROM makespacemadrid/glados-base-node:1.0

# Establece el directorio de trabajo en /app
WORKDIR /app

VOLUME /spaceapi

# Copia el archivo de requerimientos al contenedor
COPY requirements.txt .

# Copia el archivo node.py al contenedor
COPY node.py .

# Instala las dependencias
RUN pip install -r requirements.txt
USER 1000

# Ejecuta el programa cuando el contenedor se inicie
CMD ["python", "node.py"]