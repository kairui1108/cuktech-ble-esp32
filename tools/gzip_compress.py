#!/usr/bin/env python3
"""
Gzip compress CSS/JS files for ESP32 SPIFFS.
Compresses .css and .js files in place (creates .gz versions).
HTML files are served as-is (small enough).
Images are served as-is (already compressed).
"""

import gzip
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
DATA_DIR = os.path.join(PROJECT_DIR, "data")

def compress_file(filepath):
    """Gzip a single file, output to filepath.gz, then remove original."""
    with open(filepath, 'rb') as f_in:
        data = f_in.read()
    with gzip.open(filepath + '.gz', 'wb', compresslevel=9) as f_out:
        f_out.write(data)
    orig_size = len(data)
    gz_size = os.path.getsize(filepath + '.gz')
    ratio = (1 - gz_size / orig_size) * 100 if orig_size > 0 else 0
    os.remove(filepath)
    print(f"  {filepath.replace(DATA_DIR + '/', '')}: {orig_size} -> {gz_size} bytes ({ratio:.1f}% reduction, original removed)")

def main():
    print("Compressing static files for SPIFFS...")
    for root, dirs, files in os.walk(DATA_DIR):
        for fname in files:
            fpath = os.path.join(root, fname)
            # Compress CSS and JS files
            if fname.endswith('.css') or fname.endswith('.js'):
                compress_file(fpath)
    print("Done.")

if __name__ == '__main__':
    main()
