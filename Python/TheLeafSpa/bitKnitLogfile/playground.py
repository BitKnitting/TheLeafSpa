import datetime
import random
dateString = datetime.date.today().strftime('%m%d%Y')
print dateString
a = random.choice('asddabasdb')
print a
summaryFilename = dateString + random.choice('abcdefghijklmnopqrstuvwxyz')+'.csv'
print summaryFilename
