
#####################################################################
# The intent of this project is to slog through a log file generated
# from running the Leaf Spa and determine the average temperature and
# humidity when the LED is ON and then when the LED is OFF
#
from GetAvgTempHumidityAndCO2 import getAvgTempHumidityAndCO2
from WriteAveragesToLogFile import writeAveragesToLogFile
import os
# See documetation at https://docs.python.org/3/library/argparse.html
import argparse as ap
# See documentation at https://docs.python.org/2/library/logging.html
import logging
NUM_PROCESSES = 30  # Maximum number of parallel web-scraping processes.
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)
def openInputFile(inputFilename):
    if inputFilename == None:
        logging.error('Error! You must enter the path and name of the Leaf Spa log file')
        exit
    else:
        try:
            return open(inputFilename)
        except:
            logging.error('ERROR! The Logfile could not be opened')
            exit


def getUserInput():
    logger.debug('---> getUserInput()')
    parser = ap.ArgumentParser(
        description='Get the avg temp and humidity when the LEDs are ON then when the LEDs are OFF')
#     parser.add_argument('-v', '--version',
#                         action='version',
#                         version='KiCost ' + __version__)
    # See https://docs.python.org/3/library/argparse.html#name-or-flags why -i or --input
    parser.add_argument('-i', '--input',
                        # See https://docs.python.org/3/library/argparse.html#nargs
                        nargs='?',
                        type=str,
                        default=None,
                        # See https://docs.python.org/3/library/argparse.html#metavar
                        metavar='file.xml',
                        help='Full path to CSV file created by the Leaf Spa firmware.')
    return parser.parse_args()
###############################################################################
# Main entrypoint.
###############################################################################
def main():
    logger.debug('at beginning of main....')
    args = getUserInput()
    splitPath = os.path.splitext(args.input)
    outputFilename = splitPath[0]+'_analysis.csv'
    theLogFile = openInputFile(args.input)
    # it has been verified a file exists..not verified if the contents is a
    # logfile.  That is content specific, so figure this out when reading the
    # contents.
    listOfAverages = getAvgTempHumidityAndCO2(theLogFile)
    writeAveragesToLogFile(outputFilename,listOfAverages)

if __name__ == '__main__':
    main()
    print 'done'
