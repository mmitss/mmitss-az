all: 
	@echo "Usage: make [linux|clean]"

linux     : $(TGT)


%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(LIBS) -c -o $@ $<

%.o: %.cpp
	${CC} $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f $(TGT) $(LIB_NAME)*.a $(OBJ) *~ *.o
