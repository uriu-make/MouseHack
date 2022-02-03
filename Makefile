install:
	g++ src/*.cpp -o src/mousehack.elf -std=c++17
	cp src/mousehack.elf /usr/bin/mousehack
	cp mousehack.service /etc/systemd/system/mousehack.service
	systemctl enable mousehack.service
	systemctl restart mousehack.service