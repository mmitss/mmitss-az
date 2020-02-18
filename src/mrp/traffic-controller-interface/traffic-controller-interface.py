import json
from SignalController import SignalController
import time


vendorId = 0 # 0:Econolite

sigController = SignalController('10.254.56.23', 501, 0)

print(sigController.getActiveTimingPlanId())

#print(sigController.getActiveTimingPlan())

# command1 = sigController.addCommandToSchedule(6,255,5)
# print(command1)
# time.sleep(1)
# command2 = sigController.addCommandToSchedule(7,255,6)
# print(command2)
# sigController.removeCommandFromSchedule(str(command2))


# sigController.stopCommandScheduler()


# result = sigController.getActiveTimingPlan()

# j = json.dumps(result, indent=4)

# f = open("CurrentTimingPlan.json", 'w')
# f.write(j)
