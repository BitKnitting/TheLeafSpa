strs=[]
avgs = [[29.8,29.3],[31.3,31.4],[1147.312,1235.45],[None,18.2],[None,51.4],[None,574.2]]
rowToWrite = []
for count, elem in enumerate(avgs):
  for count, elem in enumerate(elem):
      try:
          print 'in try: ',count, elem
          rowToWrite[count] =  str(rowToWrite[count])+','+str(elem)
          print rowToWrite[count]
      except:
          print 'in except: ',count, elem
          rowToWrite.append(elem)
          print rowToWrite[count]
print rowToWrite[0]
print rowToWrite[1]
