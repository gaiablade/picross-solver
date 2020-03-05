COMP = /usr/bin/g++

picross-solver: source.cpp Bitmap.o
	$(COMP) $^ -o $@

Bitmap.o: Bitmap.cpp Bitmap.h
	$(COMP) $< -c -o $@
