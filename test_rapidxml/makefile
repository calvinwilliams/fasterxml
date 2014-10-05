ALL	=	press_rapidxml

all	:	$(ALL)

.cpp.o	:
	g++ -g -fPIC -Wall -Werror -fno-strict-aliasing -I. -c $<

press_rapidxml	:	press_rapidxml.o
	g++ -g -fPIC -Wall -Werror -fno-strict-aliasing -o $@ press_rapidxml.o

clean	:
	rm $(ALL) *.o

install	:

