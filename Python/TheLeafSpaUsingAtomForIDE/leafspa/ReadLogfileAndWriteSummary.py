#
#Created on Mar 8, 2017
# Margaret Johnson
#
# Assumes log file is in Settings_V format.  If the first column contains:
# - >=50 -> start or stop analyzing data.  Stop analyzing happens in the case
#   where there are multiple rows with 51 in the first column.  This means
#   the firmware was restarted.
#  - 0 ->  The row contains readings for Temp, humidity, and CO2
#  - 3 ->  The LEDs are turned ON
#  - 4 ->  The LEDs are turned OFF
#
import os
import sys
import csv
import logging
############################################################################
# Globals
logger = logging.getLogger(__name__)
#
# the summary filename is a csv where the settings and averages that came from the log file are written out to.
# I use a global variable because the summary filename is used by multiple functions.  I'm thinking there is a
# more "pythonic" way to do this..but at this point I don't have the knowledge.
#
gSummaryFilename = ""
#
# Track whether the LED light is on or off.  Sensor readings are summarized based on whether the LED was on or off.
# This is because different processes happen with there is light (e.g.: photosynthesis) and dark (e.g.: maximum respiration)
# This can be handled better.  For now, I start off with assuming the LED is on.
gLightOn = True
#
############################################################################
def setSummaryFilepath(logFileHandle):
    global gSummaryFilename # modifying gSummaryFilename so global is needed.
    splitPath = os.path.splitext(logFileHandle.name)
    gSummaryFilename = splitPath[0]+'_summary.csv'
#
# Write out what settings (e.g.: such as target CO2, etc.) the Leaf Spa is set at.  This Function
# gets called when a Settings_V record is found in the log file.
#
def writeSettingsToFile(openFileSettings,rowValueList):
    hour = ['12AM','1AM','2AM','3AM','4AM','5AM','6AM','7AM','8AM','9AM','10AM','11AM',
    '12PM','1PM','2PM','3PM','4PM','5PM','6PM','7PM','8PM','9PM','10PM','11PM']
    settingName = ['Date','Time','Seconds between readings:','Target CO2 level:','Seconds for warmup:',
    'Seconds pump is on:','Seconds between turning pump on:','Hour lights turn off:',
    'Hour lights turn on:']
    with open(gSummaryFilename, openFileSettings) as csvfile:
        writer = csv.writer(csvfile,delimiter=',')
        writer.writerow(['--Settings--'])
        for i in range(2,7):
            writer.writerow([settingName[i],rowValueList[i]])
        for i in range(7,9):
            writer.writerow([settingName[i],hour[int(rowValueList[i])]])

def writeAveragesToSummaryFile(openFileSettings):
    # listOfAverages has 6 elements: temp, humidity, CO2 (lights on)
    # temp, humidity, CO2 (lights off)
    # gone through all rows.  Write the summary info..
    # Right now this means writing out the average values...
    listOfAverages=[]
    for values in listOfValues:
        try: # perhaps there have been no sensor data for night time temps for example...
            listOfAverages.append( sum(values)/len(values))
        except:
            listOfAverages.append(0)
    averagesStringList = []
    for i, elem in enumerate(listOfAverages):
        if (i == 2 or i == 5): #CO2 values are ints
            averagesStringList.append("%d" % elem)
        else: # temp and humidity values are floats
            averagesStringList.append("%.1f" % elem)
    with open(gSummaryFilename, openFileSettings) as csvfile:
        writer = csv.writer(csvfile,delimiter=',')
        writer.writerow(['--Average Values--'])
        writer.writerow(['LED ON',' ',' ','LED OFF'])
        writer.writerow(['Temp','Humidity','CO2','Temp','Humidity','CO2'])
        writer.writerow(averagesStringList)

listOfValues = [[],[],[],[],[],[]]
def rowDoesNotExist(rowType):
    logger.error("Error! row type "+str(rowType)+ " does not exist")
def noAction(rowValueList):
    return
def settingsRow(rowValueList):
    writeSettingsToFile('wb',rowValueList)
def lightTurnedOff(rowValueList):
    global gLightOn
    gLightOn = False
def lightTurnedOn(rowValueList):
    global gLightOn
    gLightOn = True
def sensorRow(rowValueList):
    global listOfValues # modifying listOfValues[] so need the global statement
    temp = float(rowValueList[2])
    humidity = float(rowValueList[3])
    CO2 = int(rowValueList[4])
    # the listOfValues will be used to calculate the average values.  The average
    # values are maintained for when the LED is on and when it is off.  For example,
    # when the LED is off, the CO2 should be much lower because it should not be on.
    # There should be an LED row type prior to checking the state...if not, Put
    # the reading into the LED is ON group.  There should probably be a warning
    # generated....
    try:
        if gLightOn:
            listOfValues[0].append(temp)
            listOfValues[1].append(humidity)
            listOfValues[2].append(CO2)
        else:
            listOfValues[3].append(temp)
            listOfValues[4].append(humidity)
            listOfValues[5].append(CO2)
    # TBD: For now I put records that didn't get a 4 or 3 before into light on...this is not robust..
    # A better logic is:
    # The sensor data was collected when the LED was on if:
    #   a 3 record proceeds the sensor data reading.  The 3 record signifies the LED was switched on.
    #   the time is between the day on photoperiod.  This assumes a settings record was found.
    # For now perhaps reboot the Arduino to make sure the logfile has the rows.
    except:
            listOfValues[0].append(temp)
            listOfValues[1].append(humidity)
            listOfValues[2].append(CO2)
############################################################################
# switch dictionary which maps function to row type.
# see http://www.dummies.com/programming/python/how-to-replace-the-switch-statement-with-a-dictionary-in-python/
# for the technique being used.
SETTINGS_ROW = 50
switcher = {
    SETTINGS_ROW: settingsRow,
    0:sensorRow,
    3:lightTurnedOn,
    4:lightTurnedOff,
    5:noAction,
    6:noAction,
    9:noAction,
    10:noAction
}
def processRow(rowType, *args):
    return switcher.get(rowType, lambda *_: rowDoesNotExist(rowType))(*args)
############################################################################
# Entry function into the file...opens, processes rows in log file and creates
# a summary CSV file that contains any settings info as well as sensor data
# averages
############################################################################
def readLogfileAndWriteSummary(filename):
    try:
        logFileHandle = open(filename)
    except:
        logger.error('ERROR! '+filename+ ' (on the SD CARD) could not be opened.')
        sys.exit(0)
    logger.debug('at beginning of readLogfileAndWriteSummary....')
    setSummaryFilepath(logFileHandle)
    ############################################################################
    # Process the log file
    with logFileHandle as csvfile:  #the log file is a CSV file
    # all rows have a rowType.  The other fields:
    # 0 = date
    # 1 = time
    # The rest are dependent on the rowType.  To figure out the mappings, look at the functions called when
    # processRow() is called.
        dataRows = csv.DictReader(csvfile,fieldnames=('rowType','0','1','2','3','4','5','6','7','8'))
        for row in dataRows:
            # If the 'rowType' is not an integer, skip the row.
            try:
                rowType = int(row['rowType'])
            except:
                logger.error('The row type was invalid')
                continue
            # The settings version can change.
            if rowType >= SETTINGS_ROW:
                rowType = SETTINGS_ROW
            # Gather up the values into a list.  Then process the row.
            rowValueList = []
            for i in range(0,9):
                rowValueList.append(row[str(i)])
            processRow(rowType,rowValueList)
    ############################################################################
    # Write out the average values for the sensor data.
    writeAveragesToSummaryFile('ab')
    logFileHandle.close()
