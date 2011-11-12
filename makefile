snake_v2_src=trimv2.c
snake_v2_bin=snakev2

all:
		@echo "Compiling '$(snake_v2_src)..."
		gcc -o $(snake_v2_bin) $(snake_v2_src) -lrt
		@echo "Done!"
clean:
		rm -f $(snake_v2_bin)
