class TestClass(object):
    def __init__(self, data):
        self.data = data
    def print(self):
        print (self.data['name']) # Oscar 
        print (self.data['age'] + 10) # 42  