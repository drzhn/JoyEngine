import os
from os import listdir, getcwd
from os.path import isfile, join

vulkansdk = os.getenv('VK_SDK_PATH')
for filename in listdir(getcwd()):
    if os.path.isdir(filename):
        continue
    extension = os.path.splitext(filename)[1]
    if extension == ".vert" or extension == ".frag":
        comand = vulkansdk + "\Bin\glslc.exe " + filename + " -o " + filename + ".spv"
        os.system(comand)
