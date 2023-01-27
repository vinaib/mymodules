CC=arm-linux-gnueabihf-gcc
CPP=arm-linux-gnueabihf-g++
CFLAGS=-I.

#CPP=g++

#DEPS=app.o

CPPDEPS=app.o
CONTAINDEPS=container.cpp

#%.o:%.c
	#$(CC) -c $< -o $@ $(CFLAGS)

%.o:%.cpp
	$(CPP) -c $< -o $@ $(CFLAGS)

app:cppApp contain

#app_exe:$(DEPS)
	#$(CC) -o $@ $^ $(CFLAGS)
	#cp app_exe /var/lib/tftpboot

cppApp:$(CPPDEPS)
	$(CPP) -o $@ $^ 
	cp $@ /var/lib/tftpboot

contain:$(CONTAINDEPS)
	$(CPP) -o $@ $^ 

clean:
	rm -rf app_exe *.o cppApp contain
