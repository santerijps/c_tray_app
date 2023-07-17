CC:=gcc
CF:=src/*.c -Wall -Wextra -municode -lgdi32

debug:
	$(CC) $(CF) -DDEBUG -o app.debug.exe -ggdb

release:
	$(CC) $(CF) -Werror -mwindows -O3 -s -o app.release.exe
