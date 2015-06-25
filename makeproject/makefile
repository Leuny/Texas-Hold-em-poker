../target/game:../source/game.o ../source/list.o ../source/decision.o
	gcc ../source/game.o ../source/list.o ../source/decision.o -o ../target/game -lpthread
game.o:../source/game.c
	gcc -c ../source/game.c -o ../source/game.o
list.o:../source/list.c
	gcc -c ../source/list.c -o ../source/list.o
decision.o:../source/decision.c
	gcc -c ../source/decision.c -o ../source/decision.o
clean:
	rm -f ../source/*.o ../target/game