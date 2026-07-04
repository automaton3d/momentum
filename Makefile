# Makefile.nmake — build do momentum CA (MSVC via nmake)
#
# Uso:
#   nmake /f Makefile.nmake          # compila momentum.exe
#   nmake /f Makefile.nmake clean    # limpa arquivos gerados

CC = cl
CFLAGS = /Ox /W3 /std:c11

# Ajuste estes caminhos conforme sua instalação do vcpkg ou SDL3
SDL_INC = /I"E:\vcpkg\installed\x64-windows\include"
SDL_LIB = "E:\vcpkg\installed\x64-windows\lib\SDL3.lib"

all: momentum.exe

momentum.exe: momentum.c momentum.h
	$(CC) $(CFLAGS) /Fe:momentum.exe momentum.c $(SDL_INC) $(SDL_LIB)

clean:
	-del /q *.obj *.exe 2>nul

run: momentum.exe
	momentum.exe