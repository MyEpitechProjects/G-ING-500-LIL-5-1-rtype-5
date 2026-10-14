import os
import struct
import hashlib
import sys

def ensure_directories(path: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)

def add_animation(base_folder, output_path, base_texture, frames, delay):
    if isinstance(base_texture, str):
        base_texture = base_texture.encode();
    base_texture = bytearray(base_texture)
    
    f = bytearray()
    if (len(base_texture) >= 256):
        raise ValueError("Base texture path must be lower than 256");
    f.extend(struct.pack("<q", delay))
    f.extend(struct.pack("<B", len(base_texture)))
    f.extend(base_texture)
    f.extend(struct.pack("<I", len(frames)))
    for frame in frames:
        f.extend(struct.pack("<HHHH", frame["x"], frame["y"], frame["w"], frame["h"]))
    print(os.path.join(base_folder, output_path))
    ensure_directories(os.path.join(base_folder, output_path))
    with open(os.path.join(base_folder, output_path), "wb") as out:
        out.write(f)

def test_generate_test_anim():
    r = []
    for i in range(8):
        x = 0
        y = 143 * (i//2)
        if i%2 != 0:
            x = 264
        r.append({"y":y, "x":x, "w":264, "h":143})
    return r
def test_generate_test_anim2():
    r = []
    for i in range(8):
        x = 0
        y = 143 * (i//2)
        if i%2 != 0:
            x = 264
        r.append({"y":y, "x":x, "w":264, "h":143})
    return r
def test_generate_test_anim3():
    # Explosion de particules violettes (3e ligne de r-typesheet1.gif)
    FRAME_COUNT = 8
    FRAME_W = 31
    FRAME_H = 32
    START_X = 0
    START_Y = 49

    r = []
    for i in range(FRAME_COUNT):
        r.append({
            "x": START_X + FRAME_W * i,
            "y": START_Y,
            "w": FRAME_W,
            "h": FRAME_H
        })
    return r
def pack_assets(base_folder, output_pak):
    file_lists = []
    add_animation(base_folder, "images/animations/anim1.anim", "images/sheets/r-typesheet35.gif", test_generate_test_anim(), 100)
    add_animation(base_folder, "images/animations/ship/blue/stand.anim", "images/sheets/r-typesheet42.gif", [{"x":66,"y":0,"w":33, "h":17}], 200)
    add_animation(base_folder, "images/animations/ship/blue/down.anim", "images/sheets/r-typesheet42.gif", [{"x":66,"y":0,"w":33, "h":17},{"x":33,"y":0,"w":33, "h":17},{"x":0,"y":0,"w":33, "h":17}], 200)
    add_animation(base_folder, "images/animations/ship/blue/up.anim", "images/sheets/r-typesheet42.gif", [{"x":66,"y":0,"w":33, "h":17},{"x":100,"y":0,"w":33, "h":17},{"x":133,"y":0,"w":33, "h":17}], 200)
    add_animation(base_folder, "images/animations/shoot_load.anim", "images/sheets/r-typesheet1.gif", test_generate_test_anim3(), 75)


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

