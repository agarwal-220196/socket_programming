RM = rm -rf

all: proxy client

proxy: proxy.c functions.c
	gcc -I . -pthread proxy.c functions.c -o proxy

client: client.c functions.c
	mkdir client_folder
	gcc -I . client.c functions.c -o client_folder/client

clean:
	$(RM) client proxy
