# Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

"""Generate deterministic test data, with disjoint glyphs on each atlas page."""

import argparse
import struct
import zlib
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent / "Samples" / "MultiPage"
A = ["..######.."] * 2 + ["##......##"] * 4 + ["##########"] * 2 + ["##......##"] * 6
B = ["########.."] * 2 + ["##......##"] * 4 + ["########.."] * 2 + ["##......##"] * 4 + ["########.."] * 2
REPLACEMENT = [
    "##########", "##......##", "##.####.##", "##.#..#.##", "##....#.##", "##...##.##", "##..##..##",
    "##..##..##", "##......##", "##..##..##", "##..##..##", "##......##", "##......##", "##########",
]


def chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))


def make_page(glyphs):
    pixels = bytearray(64 * 16 * 4)
    for left, rows in glyphs:
        assert len(rows) == 14 and all(len(row) == 10 for row in rows)
        for y, row in enumerate(rows, start=1):
            for x, value in enumerate(row, start=left):
                if value == "#":
                    offset = (y * 64 + x) * 4
                    pixels[offset:offset + 4] = b"\xff\xff\xff\xff"
    scanlines = b"".join(b"\0" + pixels[y * 256:(y + 1) * 256] for y in range(16))
    # One stored DEFLATE block keeps bytes identical across zlib versions/platforms.
    compressed = (b"\x78\x01\x01" + struct.pack("<HH", len(scanlines), len(scanlines) ^ 0xFFFF)
                  + scanlines + struct.pack(">I", zlib.adler32(scanlines)))
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", 64, 16, 8, 6, 0, 0, 0))
            + chunk(b"IDAT", compressed) + chunk(b"IEND", b""))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Check committed bytes without writing files")
    args = parser.parse_args()
    for name, glyphs in (("Page0.png", [(0, A)]), ("Page1.png", [(12, B), (24, REPLACEMENT)])):
        expected = make_page(glyphs)
        path = ROOT / name
        if args.check:
            if not path.is_file() or path.read_bytes() != expected:
                raise SystemExit("Multi-page fixture is stale or incorrect: " + name)
        else:
            path.write_bytes(expected)
    print("Multi-page fixtures verified." if args.check else "Multi-page fixtures generated.")


if __name__ == "__main__":
    main()
