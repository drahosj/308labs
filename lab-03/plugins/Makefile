PLUGINS = echocd.so pluginmanager.so exit.so export.so pwd.so type.so
CFLAGS = -g -Wall -Werror -Wextra -Wpedantic -std=gnu99 -shared -fPIC -iquote ..

%.so: %.c
	gcc $(CFLAGS) -shared -fPIC -o $@ $<

all: $(PLUGINS)
