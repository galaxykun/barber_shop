all: clean command command_debug barber_shop barber_shop_debug

command:
	gcc command.c -o command -lrt -lpthread

command_debug:
	gcc command.c -o command_debug -D _DEBUG -lrt -lpthread

barber_shop:
	gcc barber_shop.c -o barber_shop -lrt -lpthread

barber_shop_debug:
	gcc barber_shop.c -o barber_shop_debug -D _DEBUG -lrt -lpthread

clean:
	rm -f barber_shop barber_shop_debug command command_debug