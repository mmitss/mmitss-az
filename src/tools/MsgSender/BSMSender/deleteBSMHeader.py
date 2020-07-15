def file_read(fname):
        with open (fname, "r+") as myfile:
                data=myfile.read().replace('038039', '')                
        with open(fname, 'w') as filetowrite:
            filetowrite.write(data)
                

file_read('bsmLog_fullLoop.txt')