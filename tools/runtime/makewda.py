import struct

def make_xor_key():
    seed = 23422
    key = []
    for _ in range(3418):
        seed = (seed * 725472321 + 1) & 0xFFFFFFFFFFFFFFFF
        key.append((seed >> 16) % 256)
    return key

buf = bytearray()

def w_u32(v): buf.extend(struct.pack("<I", int(v)))

# Header: tag=0x0CA7, version=1, readonly=0
buf.extend(struct.pack('<HHB', 0x0CA7, 1, 0))

# Toutes les sections à 0:
# sorts, cartes, objets, world_objects, creatures, creature_groups, teleports
for _ in range(7): w_u32(0)
# clan relations: champ inconnu + nb
w_u32(0); w_u32(0)
# clans, flags
w_u32(0); w_u32(0)
# byte final
buf.extend(b'\x00')

xor_key = make_xor_key()
encrypted = bytes(b ^ xor_key[i % 3418] for i, b in enumerate(buf))

open('WDA/T4C Worlds.WDA', 'wb').write(encrypted)
open('WDA/T4C Edit.WDA', 'wb').write(encrypted)
print(f"OK — {len(encrypted)} bytes, les deux fichiers générés")
