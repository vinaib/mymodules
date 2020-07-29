CC=arm-linux-gnueabihf-gcc
CFLAGS=-I.

DEPS=app.o

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS)

app_exe:$(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)
	cp app_exe /var/lib/tftpboot

clean:
	rm -rf app_exe *.o
