ALL:
	g++ *.cc lib/RakNet/*.cpp -I lib/ -lpthread -fpermissive -o gram -ggdb
