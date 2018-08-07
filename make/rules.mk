all: 
	@echo "Usage: make [linux|asd|rse|locomate|econolite|imx|clean]"

linux     : $(TGT)
rse       : $(TGT)
asd       : $(TGT)
locomate  : $(TGT)
econolite : $(TGT)
imx       : $(TGT)

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(LIBS) -c -o $@ $<

%.o: %.cpp
	${CC} $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f $(TGT) $(LIB_NAME)*.a $(OBJ) *~ *.o
