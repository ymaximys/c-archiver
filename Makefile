# Назва твого бінарника
TARGET = archiver
CC = gcc
CFLAGS = -Wall -Wextra -Wuninitialized -g
OBJ_DIR = build

# Знаходимо всі .c файли
SRCS = $(wildcard *.c)
# Формуємо шлях до .o файлів у папці build
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

# 1. Головна ціль
all: $(TARGET)

# 2. ПРАВИЛО ЯКОГО НЕ ВИСТАЧАЛО: Як зібрати фінальний бінарник
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# 3. Як збирати об'єктні файли в папці build
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 4. Створення папки build, якщо її нема
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# 5. Очищення
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean