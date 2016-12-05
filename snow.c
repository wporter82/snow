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
	struct _flake * prev;
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
	current->next->prev = current;
}

void draw_flakes(flake * head) {
	flake * current = head;
	while(current->next != NULL) {
		mvaddwstr(current->y, current->x, sprite[current->spriteindex]);
		current = current->next;
	}
}

void update_flakes(flake *head) {
	if(head == NULL) return;

	/* Move the flake and update it's age */
	flake * current = NULL;
	while((current = head) != NULL) {
		if(head->age > 0) {
			head->age--;
			head->y++;
		}
		head = head->next;
	}
}

void remove_dead(flake ** head) {
	flake * current = *head;

	while(current->next != NULL) {
		if(current->age < 1) {
			flake * temp = NULL;
			temp = current;

			if(temp->prev != NULL) {
				temp->prev->next = temp->next;
			} else {
				temp->next->prev = NULL;
				/* Set the new head of the list */
				*head = temp->next;
			}

			current = temp->next;
			free(temp);
		} else {
			current = current->next;
		}
	}
}

int count_list(flake *head) {
	flake *current = NULL;
	int count = 0;

	while((current = head) != NULL) {
		count++;
		head = head->next;
	}

	return count;
}

void cleanup(flake *head) {
	flake *current = NULL;
	while((current = head) != NULL) {
		head = head->next;
		free(current);
	}

}

int main(void)
{
	int maxy;
	int maxx;
	int flake_count = 0;
	int flakes_to_make = 0;
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

	/* Init linked list of flakes with a single one to start with */
	head = malloc(sizeof(flake));
	if(head == NULL) return 1;

	// Main animation loop that exits when ESC is pressed
	while(getch() != 27)
	{
		/* Update if screen size changes */
		getmaxyx(stdscr, maxy, maxx);

		/* Move existing flakes */
		update_flakes(head);
		remove_dead(&head);
		flake_count = count_list(head);

		/* Generate some new flakes if we aren't at the max */
		if(flake_count < MAX_FLAKES) {
			int x;

			/* Get random number between min and max per tick */
			flakes_to_make = MIN_FLAKES_PER_TICK + rand() / (RAND_MAX / (MAX_FLAKES_PER_TICK - MIN_FLAKES_PER_TICK + 1) + 1);
			if(flake_count + flakes_to_make > MAX_FLAKES)
				flakes_to_make = MAX_FLAKES - flake_count;

			for(x = 0; x < flakes_to_make; x++) {
				/* Get random location for x */
				int randx = rand() / (RAND_MAX / maxx + 1);
				/* Get random sprite */
				int spritenum = rand() / (RAND_MAX / num_sprites + 1);
				/* Add flake to the list */
				append_flake(head, randx, 0, spritenum, maxy);
			}
		}

		clear();
		draw_flakes(head);
		refresh();
	}

	cleanup(head); /* Cleanup particle system */
	endwin(); /* Cleanup curses */

	return 0;
}

