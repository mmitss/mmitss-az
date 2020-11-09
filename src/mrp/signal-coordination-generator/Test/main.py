from TestClass import TestClass
def main():
    d = {'name': 'Oscar', 'last_name': 'Reyes', 'age':32 }
    e = TestClass(d) 
    e.print()
    # print (e.data['name']) # Oscar 
    # print (e.data['age'] + 10) # 42 
    
if __name__ == "__main__":
    main() 