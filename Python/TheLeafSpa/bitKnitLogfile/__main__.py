
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
# See documetation at https://docs.python.org/3/library/argparse.html
import argparse as ap
# See documentation at https://docs.python.org/2/library/logging.html
import logging
NUM_PROCESSES = 30  # Maximum number of parallel web-scraping processes.
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

###############################################################################
# Main entrypoint.
###############################################################################
def main():
    #
    # file locations and such are concerningly hard coded at this point....
    #
    logger.debug('at beginning of main....')
    #
    # This script assumes it is running on a mac (i.e.: /Volumes). the SD card is named LeafSpa (i.e.: /LeafSpa)
    # and the name the arduino firmware used for the logfile name is DATALOG.TXT
    SDfilepath = "/Volumes/LeafSpa/"
    SDfilename = SDfilepath + "DATALOG.TXT"
    try:
        theLogFile = open(filename)
    except:
        logging.error('ERROR! The Logfile could not be opened')
        exit
    #
    # This is hardcoded to a location on my hard drive....
    #
    logFilespath = '/Users/margaret/Documents/LettuceBuddyHW/LadyBugHydro/TheHerbSpa/GitHub/Python/logFiles/'
    # Generate a filename based on today's date and a random character
    newFilename = datetime.date.today().strftime('%m%d%Y') + random.choice('abcdefghijklmnopqrstuvwxyz')
    # copy the log file from the SD drive to the folder holding the log files.
    logFilename = logFilespath + newFilename + '.csv'
    copyfile(SDfilename,logFilename)
    # now it's time to go through the logfile and write out summary info as well as averages...
    summaryFilename = filepath + newFilename + '_summary.csv'
    readLogfileAndWriteSummary(theLogFile,summaryFilename)
    theLogFile.close()
    # rename the "DATALOG.TXT" file on the SD drive to the filename used earlier.
    # this way, the SD card retains the older log..this is an arbitrary decision on my part...
    SDnewLogfileName = SDfilepath + newFilename + '.csv'
    os.rename(filename,SDnewLogfileName)

if __name__ == '__main__':
    main()
    print 'done'
