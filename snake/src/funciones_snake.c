#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "bits.h"

#define V 40
#define H 64
#define N 2560


#define ESCALA 10

typedef struct {
	int x,y;
	int ModX, ModY;
	char imagen;
}snk;

typedef struct {
	int x,y;
}frt;

// vector de estructuras
snk snake[N];

frt fruta;
int PAUSA = 150;
BITMAP *comida, *jugador, *cabeza;

void crear_snake() {
	int i, j;
	jugador = create_bitmap(10, 10);
	clear_bitmap(jugador);
	for(i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			putpixel(jugador, i, j, palette_color[vivora[j][i]]);
}

void crear_cabeza() {
	int i, j;
	cabeza = create_bitmap(10, 10);
	clear_bitmap(cabeza);
	for(i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			putpixel(cabeza, i, j, palette_color[cabeza_bits[j][i]]);
}

void crear_comida() {
	int i, j;
	comida = create_bitmap(10, 10);
	clear_bitmap(comida);
	for(i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			putpixel(comida, i, j, palette_color[fruta_bits[j][i]]);
}

void dibujar_bordes() {
	rect(screen, 10, 10, 630, 390, palette_color[15]);
}

void draw(char campo[V][H]) {
	int i,j;
	clear_bitmap(screen);
	dibujar_bordes();
	for(i = 0 ; i < V; i++) {
		for(j = 0; j < H ; j++) {
			if(campo[i][j] != ' ' && campo[i][j] != '%' && campo[i][j] != 'O')
				draw_sprite(screen, jugador, j * 10, i * 10);
			if(campo[i][j] == '%')
				draw_sprite(screen, comida, j * 10, i * 10);
			if(campo[i][j] == 'O')
				draw_sprite(screen, cabeza, j * 10, i * 10);
		}
	}
}

void intro_campo(char campo[V][H]) {
	int i, j;
	for(i = 0; i < V; i++)
		for(j = 0; j < H; j++)
			campo[i][j] = ' ';
}

// mete todos los datos en la matriz campo
void intro_datos(char campo[V][H], int tam) {
	int i;
	for(i = 1 ; i < tam ; i++) {
		snake[i].x = snake[i - 1].x - 1;
		snake[i].y = snake[i - 1].y;
		snake[i].imagen ='X';
	}
	snake[0].imagen = 'O';

	for(i = 0; i < tam ; i++) {
		campo[snake[i].y][snake[i].x] = snake[i].imagen;
	}
	campo[fruta.y][fruta.x] = '%';
}

void inicio(int *tam, char campo[V][H]) {
	allegro_init();
	install_keyboard();
	install_timer();
	set_gfx_mode(GFX_SAFE, 640, 400, 0, 0);

	crear_snake();
	crear_comida();
	crear_cabeza();

	// la cabeza de la serpiente
	int i;
	snake[0].x = 32;
	snake[0].y = 10;

	// tamanio de la serpiente inicial
	*tam = 4;

	// semilla de la aleatoriedad
	srand(time(NULL));

	fruta.x = rand() % (H - 1);
	fruta.y = rand() % (V - 1);

	while(fruta.x == 0) {
		fruta.x = rand() % (H - 1);
	}

	while(fruta.y == 0) {
		fruta.y = rand() % (V - 1);
	}

	for(i = 0; i < *tam ; i++) {
		snake[i].ModX = 1;
		snake[i].ModY = 0;
	}

	intro_campo(campo);
	intro_datos(campo, *tam);
}

void intro_datos_nuevos(char campo[V][H], int tam) {
	// crear la persecucion de los elementos del cuerpo a si mismo
	int i;
	for(i = tam - 1; i > 0 ; i--) {
		snake[i].x = snake[i-1].x;
		snake[i].y = snake[i-1].y;
	}
	snake[0].x += snake[0].ModX;
	snake[0].y += snake[0].ModY;
	for(i = 0 ; i < tam ; i++) {
		campo[snake[i].y][snake[i].x] = snake[i].imagen;
	}
	campo[fruta.y][fruta.x] = '%';
}

void update(char campo[V][H], int tam) {
	// borrar todos los datos de la matriz
	intro_campo(campo);
	// introducir los nuevos datos
	intro_datos_nuevos(campo, tam);
}

void input(char campo[V][H], int *tam, int *muerto) {

	// comprobar si se muere la snake
	if(snake[0].x == 0 || snake[0].x == H - 1 || snake[0].y == 0 || snake[0].y == V - 1) {
		*muerto = 1;
	}
	int i;
	for(i = 1 ; i < *tam && *muerto == 0 ; i++) {
		if(snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
			*muerto = 1;
		}
	}

	// comprobar si se come la fruta
	if(snake[0].x == fruta.x && snake[0].y == fruta.y) {
		*tam += 1;
		// regenerar la fruta y agrandar la serpiente
		snake[*tam - 1].imagen = 'X';
		PAUSA = PAUSA - 10;

		fruta.x = rand() % (H - 1);
		fruta.y = rand() % (V - 1);

		while(fruta.x == 0) {
			fruta.x = rand() % (H - 1);
		}
		while(fruta.y == 0) {
			fruta.y = rand() % (V - 1);
		}
	}
}

void loop(char campo[V][H], int tam) {
	int muerto = 0;
	int tecla = 0;
	do {
		draw(campo);
		input(campo, &tam, &muerto);
		// comprobar la tecla que se pulsa

		if(muerto == 0) {
			if(keypressed()) {
				tecla = readkey() >> 8;

				if(tecla == KEY_DOWN && snake[0].ModY != -1) {
					snake[0].ModX = 0;
					snake[0].ModY = 1;
			    }
				if(tecla == KEY_UP  && snake[0].ModY != 1) {
					snake[0].ModX = 0;
					snake[0].ModY = -1;
				}
				if(tecla == KEY_LEFT  && snake[0].ModX != 1) {
					snake[0].ModX = -1;
					snake[0].ModY = 0;
				}
				if(tecla == KEY_RIGHT  && snake[0].ModX != -1) {
					snake[0].ModX = 1;
					snake[0].ModY = 0;
				}
			}
		}
		else {
			allegro_message("GAME OVER");
		}

		update(campo, tam);
		if(PAUSA == 50) rest(50);
		else rest(PAUSA);
	} while (muerto == 0 && tecla != KEY_ESC);
}
