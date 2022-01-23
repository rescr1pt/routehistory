#  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

# ROUTEHISTORYVERSION=0.2

import time
import os
import subprocess
import threading
from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from datetime import datetime
from pytz import timezone

# usage applications
webDriverFullPath = './chromedriver.exe'
pngQuantPath = './pngquant.exe'

# constants
xpathTotalTime = ".//div[@class='auto-route-snippet-view__route-title-primary']"
xpathReversePath = ".//div[@class='route-form-view__reverse']"
moscowTz = timezone('Europe/Moscow')
runPeriod = 1200
jobCount = 1

def saveScreenshot(driver, imagePath):
    driver.save_screenshot(imagePath)
    
    # resize 
    #img = Image.open(imagePath) # image extension *.png,*.jpg
    # 1024x768
    #img = img.resize((1280, 720), Image.ANTIALIAS)
    #img.save(imagePath)
    #img.close()

    #compress
    compressArgs = pngQuantPath + ' -f --ext .png --quality 20 ' + imagePath
    subprocess.call(compressArgs, shell=False)
    
def parseMap(driver, curPath, mapUrl):
    # open link
    driver.get(mapUrl)
    time.sleep(4)
    
    mscTime = datetime.now(moscowTz)
    dateStr= str(mscTime.strftime('%Y%m%d'))
    timeStr= str(mscTime.strftime('%H%M%S'))
    
    resultPath = 'parsed/' + curPath + '/' + dateStr + '/'
    
    os.makedirs(resultPath, exist_ok=True)
    

    # get total and save screenshot
    elem = driver.find_element_by_xpath(xpathTotalTime)
    directTime = elem.text
    saveScreenshot(driver, resultPath + '/' + timeStr + '_d' + '.png')

    # reverse 
    rElem = driver.find_element_by_xpath(xpathReversePath)
    rElem.click()
    time.sleep(4)

    # get total and save screenshot
    elem = driver.find_element_by_xpath(xpathTotalTime)
    reverseTime = elem.text
    saveScreenshot(driver, resultPath + '/' + timeStr + '_r' + '.png')

    # open file and save 
    f = open(resultPath + '/' + timeStr + '.txt', "w", encoding='utf-8')
    f.write('direct=' + directTime + '\n')
    f.write('reverse=' + reverseTime + '\n')
    f.close()

def runParseMap(ways):
    options = webdriver.ChromeOptions()
    options.add_experimental_option('excludeSwitches', ['enable-logging'])
    options.binary = webDriverFullPath
    options.add_argument("--start-maximized")
    #options.add_extension('extension_3_11_2_0.crx')

    driver = webdriver.Chrome(options=options)
        
    for wayPair in ways:
        curPath = wayPair[0]
        mapUrl = wayPair[1]
        
        print('Parse: ' + curPath + '\n');
                
        parseMap(driver, curPath, mapUrl)
        
    driver.close()  

def processConfig(configName):
    f = open(configName, 'r')
    ways = f.readlines()
    
    threadWays = [[] for i in range(jobCount)]
    curJob = 0
    
    for way in ways:
        if not way:
            continue
              
        curPathPos = way.find('=')
        curPath = way[0:curPathPos]
        mapUrl = way[(curPathPos+1):]
        
        if curJob >= jobCount:
            curJob = 0
            
        pathUrls = [curPath, mapUrl]        
        
        threadWays[curJob].append(pathUrls)
        curJob+=1
    
    threads = []
    
    for nextJob in threadWays:
        thread = threading.Thread(target=runParseMap, args=(nextJob, ))
        threads.append(thread)
        thread.start()
        
    for thread in threads: 
        thread.join()  

while True:        
    mscTime = datetime.now(moscowTz)
    dateTime = str(mscTime.strftime('%Y-%m-%d %H:%M:%S'))
    
    print(dateTime + ' Start processing\n');
    
    try:
        processConfig('ways.conf')
    except Exception as e:
        print('Error - ' + str(e))
        
    dateTime = str(mscTime.strftime('%Y-%m-%d %H:%M:%S'))
    
    print(dateTime + ' Wait processing (' + str(runPeriod / 60) +' min)\n');
    
    time.sleep(runPeriod)
