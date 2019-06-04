# this file contains the general make rules to be included in module specific makefiles

# what to build in the form of ....
#
#target: prerequisites …
#(tab)  recipe         ...there must be a tab before the recipe or it will not work
# 
#the recipe can be a compiler call or a shell command
#see: https://www.gnu.org/software/make/manual/html_node/index.html#SEC_Contents 

#now we list the dependancies to build

all: 
	@echo "Usage: make [linux|clean]"

linux: $(TARGET)

#the .o's depend on the .cpp's so we compile -- % means each .o depends on the .cpp of the same name
#see https://www.gnu.org/software/make/manual/html_node/Pattern-Examples.html#Pattern-Examples
%.o: %.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@ 

#the executable depends on the .o's so we link
$(TARGET): $(OBJECTS)
	$(CPP) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBS)

.PHONY : clean
clean:
	rm -f $(TARGET) $(OBJECTS) *.out
