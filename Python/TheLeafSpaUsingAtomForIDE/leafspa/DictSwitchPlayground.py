import csv
#######################################################
# This example switches between options that invoke
# a function with different number of arguments - depending
# on the option
#######################################################
def settingsRow(secsBtwn1,secsBtwn2):
    print secsBtwn1 +"," + secsBtwn2
def sensorRow(temp,humidity,CO2):
    print 'Temperature: '+temp + ','+ humidity+','+CO2
def rowDoesNotExist(rowType):
    print "ERROR! Row Type " + str(rowType)+" does not exist"

switcher = {
    0: sensorRow,
    53: settingsRow
}
def sensorRow1(aList):
    print aList[0] +"," + aList[1] + ","+aList[2]+ ","+aList[3]+ ","+aList[4]
def settingsRow1(aList):
    print aList[0] +"," + aList[1]+","+aList[2]+","+aList[3]+","+aList[4]+","+aList[5]
switcher1 = {
    0: sensorRow1,
    53: settingsRow1
}
def processRow(rowType, *args):
    return switcher.get(rowType, lambda *_: rowDoesNotExist(rowType))(*args)
processRow(0,'22.1','70.4','1201')
# row type of 20 doesn't exist..
processRow(20,'22.1','70.4','1201')
processRow(53,'10','180')
def processRowUsingAList(rowType,aList):
    return switcher1.get(rowType,lambda *_:rowDoesNotExist(rowType))(aList)
##################################################
# This example uses a list to send in a variable number of arguments.
#
##################################################
aList = []
# Using a test file with two rows similar to a LeafSpa data file with rows 0 and 53
logFile = open("/Users/margaret/Documents/LettuceBuddyHW/LadyBugHydro/TheHerbSpa/GitHub/Python/TheLeafSpaUsingAtomForIDE/leafspa/test.csv")
with logFile as csvfile:
    dataRows = csv.DictReader(csvfile,fieldnames=('rowType','0','1','2','3','4','5','6','7','8'))
    # Look at each row
    for row in dataRows:
        # first thing, put the fields into a list.
        for i in range(0,8):
            aList.append(row[str(i)])
        # fields are in a list, so time to process the row.
        processRowUsingAList(int(row['rowType']),aList)
#        print aList[1]
#        print len(aList)
#        print aList
        #print aList
        aList = []


#    for row in csvReader:
#        t = row['info1']
#        print t
    #aList.append(row['info1'])
