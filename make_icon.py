#!/usr/bin/env python3
"""Generate resources/app.ico — medical cross icon, 3 sizes."""
import struct, os

def bmp_data(w, h, pixels):
    hdr = struct.pack('<IiiHHIIiiII', 40, w, h*2, 1, 24, 0, 0, 0, 0, 0, 0)
    xor = bytearray()
    for y in range(h-1, -1, -1):
        row = bytearray()
        for x in range(w):
            r, g, b = pixels[y*w+x]
            row += bytes([b, g, r])
        while len(row) % 4: row += b'\x00'
        xor += row
    mask_row = ((w+31)//32)*4
    return hdr + xor + bytearray(mask_row*h)

def draw(w, h):
    bg, cr = (30,58,138), (255,255,255)
    px = []
    for y in range(h):
        for x in range(w):
            bw = max(2, w//8); bh = max(2, h//8)
            cx, cy = w//2, h//2
            iv = abs(x-cx)<bw and abs(y-cy)<h*3//8
            ih = abs(y-cy)<bh and abs(x-cx)<w*3//8
            px.append(cr if iv or ih else bg)
    return px

os.makedirs('resources', exist_ok=True)
sizes = [16, 32, 48]
images = [(s, bmp_data(s, s, draw(s, s))) for s in sizes]
ico = bytearray(struct.pack('<HHH', 0, 1, len(images)))
off = 6 + len(images)*16
dirs = bytearray()
for s, bmp in images:
    dirs += struct.pack('<BBBBHHII', s, s, 0, 0, 1, 24, len(bmp), off)
    off += len(bmp)
ico += dirs
for _, bmp in images: ico += bmp
path = 'resources/app.ico'
with open(path, 'wb') as f: f.write(ico)
print(f'Created {path} ({len(ico)} bytes, sizes: {sizes})')
