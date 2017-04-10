
#####################################################################
# The intent of this project is to slog through a log file generated
# from running the Leaf Spa and determine the average temperature and
# humidity when the LED is ON and then when the LED is OFF
#
from ReadLogfileAndWriteSummary import readLogfileAndWriteSummary
import datetime
import random
from shutil import copyfile
import os
import sys
# See documentation at https://docs.python.org/2/library/logging.html
import logging
FORMAT = 'In: %(filename)s Line: %(lineno)d  %(message)s'
logging.basicConfig(level=logging.ERROR,format=FORMAT)
logger = logging.getLogger(__name__)

###############################################################################
# Main entrypoint.
# Here is where I focus on user input - including file paths and names.
# It is also the main entry point into the script that evaluates the
# log data that came from the Leaf Spa.
###############################################################################
def main():
    #
    # file locations and such are concerningly hard coded at this point....
    #
    logger.debug('at beginning of main....')
    #
    # WARNING - A HARDCODED METHOD - NOT IDEAL.
    # This script assumes it is running on a mac (i.e.: /Volumes). the SD card is named LeafSpa (i.e.: /LeafSpa)
    # and the name the arduino firmware used for the logfile name is DATALOG.TXT
    #TEST
    #SDfilepath = "/Users/margaret/Documents/LettuceBuddyHW/LadyBugHydro/TheHerbSpa/GitHub/Python/TheLeafSpaUsingAtomForIDE/leafspa/"
    SDfilepath = "/Volumes/LeafSpa/"
    SDfilename = SDfilepath + "DATALOG.TXT"
    #
    # Copy the log file on the SD drive to "permanent" storage
    # TBD: must be a better way than hardcoded...moving on for now.
    permanentLogFilePath = '/Users/margaret/Documents/LettuceBuddyHW/LadyBugHydro/TheHerbSpa/GitHub/logFiles/'
    # Generate a filename based on today's date and a random character
    newFilename = datetime.date.today().strftime('%m%d%Y') + random.choice('abcdefghijklmnopqrstuvwxyz')
    # copy the log file from the SD drive to the folder holding the log files.
    permanentLogFilename = permanentLogFilePath + newFilename + '.csv'
    try:
        copyfile(SDfilename,permanentLogFilename)
        print("Log filename: "+permanentLogFilename)
    except:
        logger.error('ERROR! '+SDfilename+' could not be copied to '+permanentLogFilename+'. Does '+SDfilename+' exist? Is it possible for a file named '+permanentLogFilename+' to exist?')
        sys.exit(0)
    readLogfileAndWriteSummary(permanentLogFilename)
# rename the "DATALOG.TXT" file on the SD drive to the filename used earlier.
# this way, the SD card retains the older log..this is an arbitrary decision on my part...
    SDnewLogfileName = SDfilepath + newFilename + '.csv'
    try:
        os.rename(SDfilename,SDnewLogfileName)
    except:
        logger.error('ERROR! '+SDfilename+' could not be renamed to '+SDnewLogfileName+'.')
        sys.exit(0)

if __name__ == '__main__':
    main()
    print 'done'
