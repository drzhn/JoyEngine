import os
from os import listdir, getcwd
import subprocess

vulkanSdk = os.getenv('VK_SDK_PATH')

print("Joy shader builder")
print()

for filename in listdir(getcwd()):
    if os.path.isdir(filename):
        continue
    extension = os.path.splitext(filename)[1]
    if extension == ".vert" or extension == ".frag":
        command = vulkanSdk + "\Bin\glslc.exe " + filename + " -o " + filename + ".spv"
        proc = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
        (out, err) = proc.communicate()
        if err is None:
            print(filename + ": OK")

print()
print("Done. Press ENTER to continue")
input()
