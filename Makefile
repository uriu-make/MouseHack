install:
	g++ src/*.cpp -o src/mousehack.o
	cp src/mousehack.o /usr/bin/mousehack
	cp mousehack.service /etc/systemd/system/mousehack.service
	systemctl enable mousehack.service
	systemctl restart mousehack.service