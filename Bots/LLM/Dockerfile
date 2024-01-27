# Utiliza una imagen base que ya tiene el cliente mqtt
FROM makespacemadrid/glados-base-node:1.0


VOLUME [ "/data" ]
# Establece el directorio de trabajo en /app
WORKDIR /app
# Copia el archivo de requerimientos al contenedor
COPY requirements.txt .
RUN pip install -r requirements.txt

# Copia el archivo node.py al contenedor
COPY node.py .
COPY llm.py .
COPY GladosIA.py .
COPY langchain.txt .
COPY UserContext.py .

#COPY .env .

USER 1000

# Ejecuta el programa cuando el contenedor se inicie
CMD ["python", "node.py"]