# Usar una imagen base de Python 3
FROM python:3.12-slim

# Establecer una carpeta de trabajo en el contenedor
WORKDIR /app

# Copiar el archivo de requisitos y instalar dependencias
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copiar el resto del código del bot al contenedor
COPY . .

# Comando por defecto para ejecutar el bot
CMD ["python", "main.py"]
