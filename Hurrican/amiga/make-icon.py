#!/usr/bin/env python3
"""Builds Hurrican.info, a Workbench tool icon with GlowIcons (OS 3.5+)
images, from Hurrican.ico.

The file holds a 4-colour planar image for older Workbenches and an IFF
FORM ICON with two 256-colour images: the helmet, and the helmet with an
orange glow for the selected state.

Usage: make-icon.py <Hurrican.ico> <Hurrican.info>   (needs Pillow)
"""
import struct
import sys

from PIL import Image, ImageChops, ImageFilter

SIZE = 48          # helmet size taken from the .ico
PAD = 3            # room for the glow
W = H = SIZE + 2 * PAD
STACK = 512 * 1024
GLOW = (255, 150, 30)

# Workbench 2.x+ default colours for the classic image
WB_PALETTE = [(170, 170, 170), (0, 0, 0), (255, 255, 255), (102, 136, 187)]


def load_helmet(path):
    ico = Image.open(path)
    ico.size = max(ico.ico.sizes())
    ico.load()
    helmet = ico.convert('RGBA').resize((SIZE, SIZE), Image.Resampling.LANCZOS)
    canvas = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    canvas.paste(helmet, (PAD, PAD))
    return canvas


def glowing(img):
    """The image inside an opaque halo, yellow at the outline fading to
    dark orange outside (GlowIcons have no partial transparency)."""
    alpha = img.getchannel('A').point(lambda a: 255 if a >= 128 else 0)
    halo = alpha.filter(ImageFilter.MaxFilter(2 * PAD + 1)).filter(ImageFilter.GaussianBlur(1.5))
    out = Image.new('RGBA', img.size, (0, 0, 0, 0))
    px = out.load()
    for y in range(img.height):
        for x in range(img.width):
            v = halo.getpixel((x, y))
            if v >= 40:
                t = (v - 40) / 215
                px[x, y] = (255, int(90 + 150 * t), int(20 * t), 255)
    out.alpha_composite(img)
    return out


def quantize(img, colours=255):
    """Palette image with index 0 transparent and up to 255 colours."""
    mask = [a >= 128 for a in img.getchannel('A').getdata()]
    rgb = img.convert('RGB').quantize(colours, method=Image.Quantize.MEDIANCUT)
    pal = rgb.getpalette()[:colours * 3]
    pixels = [(p + 1) if m else 0 for p, m in zip(rgb.getdata(), mask)]
    used = max(pixels) + 1
    palette = [(0, 0, 0)] + [tuple(pal[i * 3:i * 3 + 3]) for i in range(used - 1)]
    return pixels, palette


class BitWriter:
    def __init__(self):
        self.out = bytearray()
        self.acc = 0
        self.bits = 0

    def put(self, value, nbits):
        self.acc = (self.acc << nbits) | (value & ((1 << nbits) - 1))
        self.bits += nbits
        while self.bits >= 8:
            self.bits -= 8
            self.out.append((self.acc >> self.bits) & 0xFF)

    def data(self):
        if self.bits:
            self.out.append((self.acc << (8 - self.bits)) & 0xFF)
            self.bits = 0
        return bytes(self.out)


def rle(values, depth):
    """GlowIcons ByteRun1 over a bit stream of depth-bit values."""
    w = BitWriter()
    i, n = 0, len(values)
    while i < n:
        run = 1
        while i + run < n and run < 128 and values[i + run] == values[i]:
            run += 1
        if run >= 2:
            w.put(257 - run, 8)
            w.put(values[i], depth)
            i += run
            continue
        start = i
        while i < n and i - start < 128:
            if i + 1 < n and values[i + 1] == values[i]:
                break
            i += 1
        w.put(i - start - 1, 8)
        for v in values[start:i]:
            w.put(v, depth)
    return w.data()


def chunk(name, data):
    pad = b'\0' if len(data) & 1 else b''
    return name + struct.pack('>L', len(data)) + data + pad


def imag(pixels, palette):
    depth = max(1, (len(palette) - 1).bit_length())
    image = rle(pixels, depth)
    pal = bytes(c for rgb in palette for c in rgb)
    head = struct.pack('>BBBBBBHH',
                       0,                      # transparent colour
                       len(palette) - 1,       # number of colours - 1
                       0x03,                   # transparency + palette present
                       1, 0,                   # RLE image, raw palette
                       depth,
                       len(image) - 1, len(pal) - 1)
    return chunk(b'IMAG', head + image + pal)


def glow_form(images):
    max_pal = max(len(p) for _, p in images) * 3
    face = struct.pack('>BBBBH', W - 1, H - 1, 0, 0x11, max_pal - 1)
    body = b'ICON' + chunk(b'FACE', face) + b''.join(imag(px, pal) for px, pal in images)
    return b'FORM' + struct.pack('>L', len(body)) + body


def planar(img):
    """Two bitplanes in the Workbench colours."""
    pal_img = Image.new('P', (1, 1))
    pal_img.putpalette([c for rgb in WB_PALETTE for c in rgb] + [0] * (256 - 4) * 3)
    flat = Image.new('RGB', img.size, WB_PALETTE[0])
    flat.paste(img.convert('RGB'), mask=img.getchannel('A').point(lambda a: 255 if a >= 128 else 0))
    idx = list(flat.quantize(palette=pal_img, dither=Image.Dither.NONE).getdata())
    words = (W + 15) // 16
    planes = bytearray()
    for plane in range(2):
        for y in range(H):
            for wd in range(words):
                v = 0
                for b in range(16):
                    x = wd * 16 + b
                    if x < W and (idx[y * W + x] >> plane) & 1:
                        v |= 0x8000 >> b
                planes += struct.pack('>H', v)
    return struct.pack('>hhhhhLBBL', 0, 0, W, H, 2, 1, 3, 0, 0) + bytes(planes)


def disk_object(normal, selected):
    gadget = struct.pack('>LhhhhHHHLLLlLHL',
                         0, 0, 0, W, H,
                         0x0006,      # GFLG_GADGIMAGE | GFLG_GADGHIMAGE
                         0x0003,      # RELVERIFY | GADGIMMEDIATE
                         0x0001,      # BOOLGADGET
                         1, 1, 0,     # GadgetRender, SelectRender present
                         0, 0, 0,
                         1)           # UserData: OS 2.x icon revision
    head = struct.pack('>HH', 0xE310, 1) + gadget + struct.pack(
        '>BBLLllLLl',
        3, 0,                         # WBTOOL
        0, 0,                         # no default tool, no tool types
        -0x80000000, -0x80000000,     # NO_ICON_POSITION
        0, 0, STACK)
    return head + planar(normal) + planar(selected)


def main(src, dst):
    helmet = load_helmet(src)
    selected = glowing(helmet)
    with open(dst, 'wb') as f:
        f.write(disk_object(helmet, selected))
        f.write(glow_form([quantize(helmet), quantize(selected)]))


if __name__ == '__main__':
    main(*sys.argv[1:3])
