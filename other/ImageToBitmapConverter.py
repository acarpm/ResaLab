from PIL import Image

def image_to_c_array(image_path, output_path="bitmap.h"):
    img = Image.open(image_path).convert("L")  # conversion en niveaux de gris
    img = img.point(lambda x: 0 if x < 128 else 1, '1')  # binarisation
    pixels = list(img.getdata())
    width, height = img.size

    bytes_per_row = (width + 7) // 8
    data = []

    for y in range(height):
        byte = 0
        bit_count = 0
        for x in range(width):
            bit = pixels[y * width + x]
            byte = (byte << 1) | bit
            bit_count += 1
            if bit_count == 8:
                data.append(byte)
                byte = 0
                bit_count = 0
        if bit_count > 0:
            byte <<= (8 - bit_count)
            data.append(byte)

    with open(output_path, "w") as f:
        f.write(f"const uint8_t bitmap[{len(data)}] = {{\n")
        for i, b in enumerate(data):
            f.write(f"0x{b:02X}, ")
            if (i + 1) % 12 == 0:
                f.write("\n")
        f.write("\n};\n")
        f.write(f"// width: {width}, height: {height}\n")

    print(f"Bitmap généré : {output_path}")

image_to_c_array("images/CampusFab_Logo.png")
# Exemple d’utilisation :
# image_to_c_array("mon_image.png")
