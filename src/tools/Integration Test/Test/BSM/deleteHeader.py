def file_read(fname):
        with open (fname, "r+") as myfile:
                data=myfile.read().replace('038039', '')                
        with open(fname, 'w') as filetowrite:
            filetowrite.write(data)
                

file_read('Mountain_Campbell_Westbound.txt')
# file_read('MountainCampbell_Eastbound.txt')
# with open('MountainCampbell_Eastbound.txt', 'r') as myfile:
#   data = myfile.read().replace('038039', '')
# print(data)