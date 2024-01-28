"""
Converts glyphs in the current directory to a C header.

Usage: python -m fonttool font_24.h 24
"""

import os
import re
import sys
import subprocess

dst = sys.argv[1]
font_size = sys.argv[2]
header = "FONT_{}_H".format(font_size)
files = [f for f in os.listdir('.') if os.path.isfile(f) and f.endswith(".jpg")]
files.sort()

with open(dst, "w") as o:
    o.write("#ifndef {}\n".format(header))
    o.write("#define {}\n\n".format(header))

    for i, src in enumerate(files):    
        id = src.split('.')[0]
    
        rv = subprocess.run(
            ["identify", "-ping", "-format", "%w %h", src],
            capture_output=True,
            text=True
        )
        
        dim = rv.stdout.split(' ')
        width = int(dim[0])
        height = int(dim[1])

        print("Image={}, size={}x{}".format(src, width, height))
    
        if i == 0:
            o.write("int font_{0}_height = {1};\n\n".format(font_size, height))
            
        subprocess.run(["convert", src, "-threshold", "80%", "mon.jpg"])
        subprocess.run(["convert", "mon.jpg", "-depth", "1", "-format", "'txt'", "mon.txt"])
    
        n = 7
        x = 0
        column_len = 0
        byte_count = 0
    
        res = width * height
        image_size = (res // 8) + (res % 8 > 0) 
    
        o.write("int {0}_width = {1};\n\n".format(id, width))
        o.write("unsigned char {0}_bmp[{1}] = {{\n\t".format(id, image_size))
        
        with open("mon.txt", "r") as fd:
            fd.readline()
            for line in fd:
                px = re.search("\([^\)]+\)", line).group()
                if px == "(0)":                        
                    x |= (1 << n)

                n -= 1

                if n < 0:
                    o.write("0x{:02X}, ".format(x))

                    column_len += 1
                    byte_count += 1

                    if column_len >= 12:
                        o.write("\n\t")
                        column_len = 0

                    x = 0
                    n = 7

            if res % 8 != 0:
                o.write("0x{:02X}, ".format(x))
                byte_count += 1

        o.write("\n};\n\n")

        if (image_size != byte_count):
            raise Exception("Detected image size {} and bytes written {} not equal".format(image_size, byte_count))
                

    o.write("#endif")
        
os.remove("mon.jpg")
os.remove("mon.txt")
    
print("Wrote {} bytes of image data to {}".format(byte_count, dst))
