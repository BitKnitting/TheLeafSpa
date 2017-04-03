#
gFilename = ""
def makeName():
    global gFilename
    playgroundFolder = '/Users/margaret/Documents/LettuceBuddyHW/LadyBugHydro/TheHerbSpa/GitHub/Python/Playground/'
    # Test file
    testFilename = 'TestFile.csv'
    gFilename = playgroundFolder + testFilename
    fileHandle = open(gFilename)
    print 'in makeName ' + fileHandle.name
def main():
    makeName()
    print 'in main ' + gFilename
if __name__ == '__main__':
    main()
    print 'done'
