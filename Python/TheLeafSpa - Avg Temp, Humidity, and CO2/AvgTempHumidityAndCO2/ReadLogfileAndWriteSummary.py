#
#Created on Mar 8, 2017
# Margaret Johnson
#
# Assumes log file is in Settings_V1 format.  If the first column contains:
# - 51 -> start or stop analyzing data.  Stop analyzing happens in the case
#   where there are multiple rows with 51 in the first column.  This means
#   the firmware was restarted.
#  - 0 ->  The row contains readings for Temp, humidity, and CO2
#  - 3 ->  The LEDs are turned ON
#  - 4 ->  The LEDs are turned OFF
#

import sys
import csv
import logging
logger = logging.getLogger(__name__)
def writeSettingsToFile(openFileSettings,summaryFilename,secsBtwnReadings,targetCO2Level,secsWarmUp,amtSecsPumpIsOn,
                        secsBtwnTurningPumpOn,hourToTurnLightOff,hourToTurnLightOn):
    hour = ['12AM','1AM','2AM','3AM','4AM','5AM','6AM','7AM','8AM','9AM','10AM']
    with open(summaryFilename, openFileSettings) as csvfile:
        writer = csv.writer(csvfile,delimiter=',')
        writer.writerow(['--Settings--'])
        writer.writerow(['Seconds between readings:',secsBtwnReadings])
        writer.writerow(['Target CO2 level:',targetCO2Level])
        writer.writerow(['Seconds for warmup:',secsWarmUp])
        writer.writerow(['Seconds pump is on:',amtSecsPumpIsOn])
        writer.writerow(['Seconds between turning pump on:',secsBtwnTurningPumpOn])
        writer.writerow(['Hour lights turn off:',hour[int(hourToTurnLightOff)]])
        writer.writerow(['Hour lights turn on:',hour[int(hourToTurnLightOn)]])

def writeAveragesToSummaryFile(openFileSettings,summaryFilename,listOfAverages):
    # listOfAverages has 6 elements: temp, humidity, CO2 (lights on)
    # temp, humidity, CO2 (lights off)
    averagesStringList = []
    for i, elem in enumerate(listOfAverages):
        if (i == 2 or i == 5): #CO2 values are ints
            averagesStringList.append("%d" % elem)
        else: # temp and humidity values are floats
            averagesStringList.append("%.1f" % elem)
    with open(summaryFilename, openFileSettings) as csvfile:
        writer = csv.writer(csvfile,delimiter=',')
        writer.writerow(['--Average Values--'])
        writer.writerow(['LED ON',' ',' ','LED OFF'])
        writer.writerow(['Temp','Humidity','CO2','Temp','Humidity','CO2'])
        writer.writerow(averagesStringList)
#
# Returns the average values as a list of lists: [ [results for first run],
# [results for second run]...]
# results list:
# Average temp when LED is ON
# Average humidity when LED is ON
# Average CO2 when LED is ON
# Average temp when LED is OFF
# Average humidity when LED is OFF
# Average CO2 when LED is OFF
def readLogfileAndWriteSummary(logFile,summaryFilename):
    logger.debug('at beginning of readLogfileAndWriteSummary....')
    # There should be 6 entries in the listOfValues
    # First 3 lists are for temp,humidity,CO2 when LED is ON.
    # Second 3 for when LED is OFF.
    listOfValues = [[],[],[],[],[],[]]
    with logFile as csvfile:  #the log file is a CSV file
    # all rows have a rowType, date, and time.  The other fields are additional info.
    # Currently, Sensor Data (rowType = 0) info1 = temperature, info2 = humidity, info3 = CO2 level
    # The Settings Row (current = 53) info1 = seconds between readings, info2 = target CO2 level,
    # info3 = warmup time in seconds, info4 = number of seconds the water pump is on, info5 = number of seconds
    # between turning the pump on, info6 = the hour to turn the light off, info7 = the hour to turn the light on.
        csvReader = csv.DictReader(csvfile,fieldnames=('rowType','date','time','info1','info2','info3','info4','info5','info6','info7'))

        for row in csvReader:
            # If the 'rowType' is not an integer, skip the row.
            try:
                rowType = int(row['rowType'])
            except:
                logger.error('The row type was invalid')
                break;
            if rowType > 50: #Settings record
                writeSettingsToFile('wb',summaryFilename,row['info1'],row['info2'],row['info3'],row['info4'],row['info5'],row['info6'],row['info7'])
                continue
            if rowType == 3: #LED is turned on
                inLEDonState = True
                continue
            if rowType == 4: # LED is turned off
                inLEDonState = False
                continue
            if rowType == 0: # A reading
                temp = float(row['info1'])
                humidity = float(row['info2'])
                CO2 = float(row['info3'])
                # the listOfValues will be used to calculate the average values.  The average
                # values are maintained for when the LED is on and when it is off.  For example,
                # when the LED is off, the CO2 should be much lower because it should not be on.
                # There should be an LED row type prior to checking the state...if not, Put
                # the reading into the LED is ON group.  There should probably be a warning
                # generated....
                try:
                    if inLEDonState:
                        listOfValues[0].append(temp)
                        listOfValues[1].append(humidity)
                        listOfValues[2].append(CO2)
                    else:
                        listOfValues[3].append(temp)
                        listOfValues[4].append(humidity)
                        listOfValues[5].append(CO2)
                except:
                        listOfValues[0].append(temp)
                        listOfValues[1].append(humidity)
                        listOfValues[2].append(CO2)
    # gone through all rows.  Write the summary info..
    # Right now this means writing out the average values...
    listOfAverages=[]
    for values in listOfValues:
        try: # perhaps there have been no sensor data for night time temps for example...
            listOfAverages.append( sum(values)/len(values))
        except:
            listOfAverages.append(0)
    writeAveragesToSummaryFile('ab',summaryFilename,listOfAverages)
