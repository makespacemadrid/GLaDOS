# Utiliza una imagen base de Python
FROM makespacemadrid/glados-base-node:1.5

# Establece el directorio de trabajo en /app
WORKDIR /app

# Copia el archivo de requerimientos al contenedor
COPY requirements.txt .
# Instala las dependencias
RUN pip install -r requirements.txt

# Copia el archivo node.py al contenedor
COPY IATools.py .
COPY GladosBotBase.py .

# Ejecuta el programa cuando el contenedor se inicie
CMD ["python", "main.py"]1