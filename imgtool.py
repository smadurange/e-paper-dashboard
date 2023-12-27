"""
Converts input image to a c header with image data for
Waveshare 7.5-inch e-paper display (V2) using ImageMagick.

Usage: python -m imgtool image.jpg offset_x offset_y
"""

import os
import re
import sys
import subprocess

src = sys.argv[1]
offset_x = int(sys.argv[2])
offset_y = int(sys.argv[3])

file_name = src.replace("." + src.split('.')[-1], "")
dst = file_name + ".h"
header = file_name.upper() + "_IMG_H"

rv = subprocess.run(
    ["identify", "-ping", "-format", "%w %h", src],
    capture_output=True,
    text=True
)

dim = rv.stdout.split(' ')
width = int(dim[0])
height = int(dim[1])
print("Detected image size: {}x{}".format(width, height))

subprocess.run(["convert", src, "-threshold", "80%", "mon.jpg"])
subprocess.run(["convert", "mon.jpg", "-depth", "1", "-format", "'txt'", "mon.txt"])

n = 7
x = 0
column_len = 0
byte_count = 0

res = width * height
image_size = (res // 8) + (res % 8 > 0) 

with open(dst, "w") as o:
    o.write("#ifndef {}\n".format(header))
    o.write("#define {}\n\n".format(header))

    o.write("const int {0}_width = {1};\n".format(file_name, width))
    o.write("const int {0}_height = {1};\n\n".format(file_name, height))

    o.write("const int {0}_offset_x = {1};\n".format(file_name, offset_x))
    o.write("const int {0}_offset_y = {1};\n\n".format(file_name, offset_y))

    o.write("const unsigned char {0}_bmp[{1}] = {{\n\t".format(file_name, image_size))

    with open("mon.txt", "r") as i:
        i.readline()
        for line in i:
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
    o.write("#endif")

os.remove("mon.jpg")
os.remove("mon.txt")

if (image_size != byte_count):
    raise Exception("Detected image size {} and bytes written {} not equal".format(image_size, byte_count))

print("Wrote {} bytes of image data to {}".format(byte_count, dst))
