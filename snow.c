#define _GNU_SOURCE
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <stdlib.h>

#define REFRESH_RATE 60 /* milliseconds to wait for input */
#define MAX_FLAKES 200
#define MIN_FLAKES_PER_TICK 0
#define MAX_FLAKES_PER_TICK 5

// Linked list of flakes
typedef struct _flake {
	int x;
	int y;
	int spriteindex;
	int age;
	struct _flake * next;
} flake;

// Unicode characters to use as sprites
const wchar_t* sprite[8] = {
	L"\u2744",
	L"\u2745",
	L"\u2746",
	L"\u2747",
	L"\u2748",
	L"\u2749",
	L"\u274A",
	L"\u274B"
};

const int num_sprites = sizeof(sprite) / sizeof(sprite[0]);

void append_flake(flake * head, int x, int y, int spriteindex, int age) {
	flake * current = head;
	/* Get to the end of the list */
	while(current->next != NULL) {
		current = current->next;
	}

	current->next = malloc(sizeof(flake));
	current->next->x = x;
	current->next->y = y;
	current->next->spriteindex = spriteindex;
	current->next->age = age;
	current->next->next = NULL;
}

void draw_flakes(flake * head) {
	flake * current = head;
	while(current->next != NULL) {
		mvaddwstr(current->y, current->x, sprite[current->spriteindex]);
		/* mvaddwstr(y, x, sprite[0]); */
		current = current->next;
	}
}

void update_flakes(flake ** head) {
	if(*head == NULL) return;

	/* Check if we need to remove the first item */
	if((*head)->age < 1) {
		free(*head);
		*head = (*head)->next;
	}

	flake * current = *head;
	flake * previous = NULL;

	while(current->next != NULL) {
		if(current->age > 0) {
			current->age--;
			current->y++;
		} else {
			/* Destroy the object, it's dead to us */
			if(previous != NULL) {
				previous->next = current->next;
				free(current);
				current = current->next;
			}
		}
		previous = current;
		current = current->next;
	}
}

int count_list(flake * head) {
	flake * current = head;
	int count = 0;

	while(current->next != NULL) {
		count++;
		current = current->next;
	}

	return count;
}

int main(void)
{
	int maxy;
	int maxx;
	int flake_count = 0;
	int flakes_to_make = 0;
	int max_age;
	flake * head = NULL;

	// Init curses
	setlocale(LC_ALL, ""); /* Enable using all unicode characters */
	initscr();
	raw();
	keypad(stdscr, TRUE); /* Enable the whole keyboard */
	noecho(); /* Don't display typed characters */
	curs_set(0); /* Turn off the cursor */
	timeout(REFRESH_RATE); /* Block on getch() to pause animation */

	getmaxyx(stdscr, maxy, maxx);
	max_age = maxy;

	// Init linked list of flakes
	head = malloc(sizeof(flake));
	if(head == NULL) return 1;
	head->age = max_age;

	// Main animation loop that exits when ESC is pressed
	while(getch() != 27)
	{
		/* Update if screen size changes */
		getmaxyx(stdscr, maxy, maxx);
		max_age = maxy;

		/* Move existing flakes */
		update_flakes(&head);
		flake_count = count_list(head);

		/* Generate some new flakes */
		if(flake_count < MAX_FLAKES) {
			flakes_to_make = MIN_FLAKES_PER_TICK + rand() / (RAND_MAX / (MAX_FLAKES_PER_TICK - MIN_FLAKES_PER_TICK + 1) + 1);
			if(flake_count + flakes_to_make > MAX_FLAKES)
				flakes_to_make = MAX_FLAKES - flake_count;
			int x;
			for(x = 0; x < flakes_to_make; x++) {
				/* Get random location for x */
				int randx = rand() / (RAND_MAX / maxx + 1);
				/* Get random sprite */
				int spritenum = rand() / (RAND_MAX / num_sprites + 1);
				/* Add flake to the list */
				append_flake(head, randx, 0, spritenum, max_age);
			}
		}

		clear();
		draw_flakes(head);
		refresh();
	}

	endwin(); /* Cleanup curses */

	return 0;
}

