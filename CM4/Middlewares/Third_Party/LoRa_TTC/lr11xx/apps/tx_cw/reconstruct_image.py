import re
import binascii
import os


def reconstruct_from_log(log_file, output_image):
    if not os.path.exists(log_file):
        print(f"Error: El archivo de log '{log_file}' no existe.")
        print("Asegurate de guardar la salida de la terminal en un archivo de texto.")
        return


    print(f"Leyendo {log_file}...")
    with open(log_file, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()


    # El usuario indica que el archivo contiene directamente el código HEX.
    # Limpiamos todo lo que no sea caracteres hexadecimales (espacios, saltos de línea, etc.)
    hex_chunks = []


    for line in content.splitlines():
        if "IMGHEX:" in line:
            hex_part = line.split("IMGHEX:", 1)[1]
            hex_chunks.append(hex_part.strip())


    clean_hex = "".join(hex_chunks)


    if not clean_hex:
        print("Error: No se encontraron lineas IMGHEX validas en el log.")
        return




    print(f"Procesando {len(clean_hex)} caracteres hexadecimales...")


    try:
        # Convertir la cadena hex completa a bytes
        all_data = binascii.unhexlify(clean_hex)
    except binascii.Error as e:
        print(f"Error convirtiendo hex a binario: {e}")
        print("Posible causa: Longitud impar de caracteres hex o caracteres inválidos.")
        return


    with open(output_image, 'wb') as f:
        f.write(all_data)
   
    print(f"\n[EXITO] Imagen reconstruida guardada en: {output_image}")
    print(f"Tamaño total: {len(all_data)} bytes")


if __name__ == "__main__":
    # --- CONFIGURACION ---
    # Cambia este nombre por el nombre del archivo donde guardaste el log de la terminal
    #INPUT_LOG_FILE = "HEX.txt"
    INPUT_LOG_FILE = r"C:\Users\nilc\Desktop\imagenPAE.txt"


   
    # Nombre de la imagen de salida
    OUTPUT_IMAGE_FILE = "reconstructed_image.png"
    # ---------------------


    reconstruct_from_log(INPUT_LOG_FILE, OUTPUT_IMAGE_FILE)





