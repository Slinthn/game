import sys
from array import array

if len(sys.argv) != 3:
  print("ERROR: Syntax: py <input file> <output file>")
  exit()

inputfilename = sys.argv[1]
outputfilename = sys.argv[2]

inputfile = open(inputfilename, "rb")
outputfile = open(outputfilename, "wb")

propertycount = 0
vertexcount = 0
facecount = 0

inputfile.seek(18)
outputfile.write(inputfile.read(8)) # Width and height (u32)

while True:
  b = inputfile.read(3)
  if len(b) < 3:
    break
  b = bytearray(b)
  b[1], b[2] = b[2], b[1]
  b.append(255)
  outputfile.write(b)

print("compiled binary BMP " + outputfilename)
