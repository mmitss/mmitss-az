import datetime

currentTime = datetime.datetime.now()
timedelta = 3.5
newtime = currentTime + datetime.timedelta(seconds=timedelta)
print(currentTime)
print(newtime)