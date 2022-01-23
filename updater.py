#  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

import sys
import os
import subprocess

def compressScreenshot(imagePath):
    compressArgs = './pngquant.exe' + ' -f --ext .png --quality 20 ' + imagePath
    subprocess.call(compressArgs, shell=False)
 
path = str(sys.argv[1])
print(path)

files = os.listdir(path)

size1 = len('20210902173917_r.png')
size2 = len('20210902213320.txt')
dateSize = len('20210902')


for file in files:
    if len(file) != size1 and len(file) != size2:
        continue
        
    if len(file) == size1:
        compressScreenshot(path + '/' + file)
    
    fileDate = file[:dateSize]
    fileTime = file[dateSize:]   
    resultPath = path + '/' + fileDate 
    os.makedirs(resultPath, exist_ok=True)
    os.rename(path + '/' + file, resultPath + '/' + fileTime)