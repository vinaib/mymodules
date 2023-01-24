CC=arm-linux-gnueabihf-gcc
CPP=arm-linux-gnueabihf-g++
CFLAGS=-I.

#CPP=g++

#DEPS=app.o

CPPDEPS=app.o

#%.o:%.c
	#$(CC) -c $< -o $@ $(CFLAGS)

%.o:%.cpp
	$(CPP) -c $< -o $@ $(CFLAGS)

app:cppApp

#app_exe:$(DEPS)
	#$(CC) -o $@ $^ $(CFLAGS)
	#cp app_exe /var/lib/tftpboot

cppApp:$(CPPDEPS)
	$(CPP) -o $@ $^ 
	cp $@ /var/lib/tftpboot

clean:
	rm -rf app_exe *.o cppApp
