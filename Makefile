CC=gcc
LIBS=-lcurl
TARGET=chess
SRC=chess.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)

