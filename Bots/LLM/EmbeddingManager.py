import os
from langchain.embeddings import OpenAIEmbeddings
import json
import numpy as np


def cosine_similarity(vec1, vec2):
    """Calcula la similitud del coseno entre dos vectores."""
    dot_product = np.dot(vec1, vec2)
    norm_vec1 = np.linalg.norm(vec1)
    norm_vec2 = np.linalg.norm(vec2)
    return dot_product / (norm_vec1 * norm_vec2)


class EmbeddingManager:
    def __init__(self):
        # Inicializa el gestor de embeddings
        self.embeddings = OpenAIEmbeddings()
        self.storage = {}  # Un diccionario simple para almacenar los embeddings

    def ingest(self, file_paths):
        """ Genera y almacena embeddings para los archivos de texto proporcionados. """
        for file_path in file_paths:
            if os.path.exists(file_path) and os.path.isfile(file_path):
                with open(file_path, 'r', encoding='utf-8') as file:
                    text = file.read()
                    embedding = self.embeddings.embed_query(text)
                    self.storage[file_path] = embedding

    def search(self, query, top_n=5):
        """ Busca en los embeddings almacenados y devuelve los más similares a la consulta. """
        query_embedding = self.embeddings.embed_query(query)

        scores = {path: cosine_similarity(query_embedding, emb) for path, emb in self.storage.items()}
        sorted_scores = sorted(scores.items(), key=lambda x: x[1], reverse=True)
        return sorted_scores[:top_n]


# Ejemplo de uso
#embedding_manager = EmbeddingManager()
#embedding_manager.ingest(['path_to_text_file1.txt', 'path_to_text_file2.txt'])  # Rutas a los archivos de texto

# Realizar una búsqueda
#search_results = embedding_manager.search("Consulta de ejemplo")
#print(json.dumps(search_results, indent=4))  # Imprimir resultados formateados
