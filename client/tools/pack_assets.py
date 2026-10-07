import os
import struct
import hashlib
import sys

def pack_assets(base_folder, output_pak):
    file_lists = []
    
    for root, _, files in os.walk(base_folder):
        for file in files:
            full_path = os.path.join(root, file)
            rel_path = os.path.relpath(full_path, base_folder)
            # Normalise les séparateurs de chemin sous Windows/Linux
            file_lists.append(rel_path.replace("\\", "/"))

    file_lists.sort()

    f = bytearray()
    f.extend(struct.pack("<I", len(file_lists)))
    data = bytearray()
    offset = 0

    for rel_path in file_lists:
        full_path = os.path.join(base_folder, rel_path)
        with open(full_path, "rb") as fl:
            fdata = fl.read()
        
        data.extend(fdata)
        # 4 octets (offset), 4 octets (taille)
        f.extend(struct.pack("<II", offset, len(fdata)))
        # 8 first octets SHA-256 filename
        f.extend(hashlib.sha256(rel_path.encode('utf-8')).digest()[:8])
        offset += len(fdata)

    total_header_size = len(f) + 4
    t = bytearray(struct.pack("<I", total_header_size))
    t.extend(f)
    t.extend(data)

    with open(output_pak, "wb") as out:
        out.write(t)
    print(f"[Assets Packer] {len(file_lists)} files packed in '{output_pak}'")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 pack_assets.py <assets_dir> <output_pak>")
        sys.exit(1)
    pack_assets(sys.argv[1], sys.argv[2])

