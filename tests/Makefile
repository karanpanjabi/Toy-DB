all: create_table read_directory read_table insert_table

create_table: create_table.c ../src/*.c
	gcc -g create_table.c ../src/*.c -o create_table

read_table: read_table.c ../src/*.c
	gcc -g read_table.c ../src/*.c -o read_table

read_directory: read_directory.c ../src/*.c
	gcc -g read_directory.c ../src/*.c -o read_directory

insert_table: insert_table.c ../src/*.c
	gcc -g insert_table.c ../src/*.c -o insert_table

clean:
	rm -f *.o *.db
