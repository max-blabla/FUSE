BOT = bot
MNT = ./tmp
bot: $(BOT).c 
	gcc -Wall $(BOT).c `pkg-config fuse3 --cflags --libs` -o $(BOT)
	
make run:
	./$(BOT) $(MNT)  
