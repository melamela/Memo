object = CommonFuc.o \
	 config.o \
	 Processer.o \
	 Sender.o \
	 SocketClient.o \
	 task_base.o 
INCLUDE = -I.


%.o:%.cpp
	g++ -o2 -D_SHOW_DEBUG_ -o $@ -c $< $(INCLUDE)


all:overload.a overload


overload.a:$(object)
	ar -crvs $@ $^


overload:main.o overload.a
	g++ -o2 -o overload main.o overload.a -lpthread

clean:
	rm -f $(object)
	rm -f *.o
	rm -f overload overload.a


