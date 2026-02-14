import struct
import os
import sys

def extract_big(file_path, output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        
    with open(file_path, 'rb') as f:
        header = f.read(4)
        if header not in [b'BIG4', b'BIGF']:
            print(f"Not a BIG file: {header}")
            return
            
        file_size = struct.unpack('<I', f.read(4))[0]
        num_files = struct.unpack('>I', f.read(4))[0] # BIG files often use BE for num_files
        header_size = struct.unpack('>I', f.read(4))[0]
        
        print(f"Extracting {num_files} files from {file_path}...")
        
        entries = []
        for _ in range(num_files):
            offset = struct.unpack('>I', f.read(4))[0]
            size = struct.unpack('>I', f.read(4))[0]
            name = b''
            while True:
                char = f.read(1)
                if char == b'\0' or not char:
                    break
                name += char
            entries.append((offset, size, name.decode('ascii', errors='ignore')))
            
        for offset, size, name in entries:
            # Handle directory separators in BIG names
            name = name.replace('\\', os.sep)
            out_path = os.path.join(output_dir, name)
            out_subdir = os.path.dirname(out_path)
            if not os.path.exists(out_subdir):
                os.makedirs(out_subdir)
                
            f.seek(offset)
            data = f.read(size)
            with open(out_path, 'wb') as out_f:
                out_f.write(data)
            # print(f"Extracted: {name}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python extract_big.py <input.big> <output_dir>")
    else:
        extract_big(sys.argv[1], sys.argv[2])
