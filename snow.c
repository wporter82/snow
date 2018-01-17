#define _GNU_SOURCE
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <stdlib.h>

#define REFRESH_RATE 60 /* milliseconds to wait for input */
#define MAX_FLAKES 200
#define MIN_FLAKES_PER_TICK 0
#define MAX_FLAKES_PER_TICK 5
#define MAX_SPEED 2

// Linked list of flakes
typedef struct flake {
	int x;
	int y;
	int spriteindex;
	int age;
	int speed;
	struct flake* next;
} flake;

// Callback to manipulate nodes of the linked list
typedef void (*callback)(flake* data);

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

flake* create(int x, int y, int spriteindex, int age, int speed, flake* next) {
	flake* new_flake = (flake*)malloc(sizeof(flake));
	if(new_flake == NULL) {
		printf("Error creating a new flake.\n");
		exit(0);
	}
	new_flake->x = x;
	new_flake->y = y;
	new_flake->spriteindex = spriteindex;
	new_flake->age = age;
	new_flake->speed = speed;
	new_flake->next = next;

	return new_flake;
}

flake* prepend(flake* head, int x, int y, int spriteindex, int age, int speed) {
	flake* new_flake = create(x, y, spriteindex, age, speed, head);
	head = new_flake;
	return head;
}

flake* append(flake* head, int x, int y, int spriteindex, int age, int speed) {
	/* Get to the end of the list */
	flake* cursor = head;
	while(cursor->next != NULL) {
		cursor = cursor->next;
	}

	/* Create a new flake */
	flake* new_flake = create(x, y, spriteindex, age, speed, NULL);
	cursor->next = new_flake;

	return head;
}

flake* remove_front(flake* head) {
	if(head == NULL)
		return NULL;
	flake *front = head;
	head = head->next;
	/* check if last in list */
	if(front == head)
		head = NULL;
	free(front);
	return head;
}

flake* remove_back(flake* head) {
	if(head == NULL)
		return NULL;

	flake *cursor = head;
	flake *back = NULL;
	while(cursor->next != NULL) {
		back = cursor;
		cursor = cursor->next;
	}
	if(back != NULL)
		back->next = NULL;

	/* check if last in list */
	if(cursor == head)
		head = NULL;

	free(cursor);
	return head;
}

flake* remove_any(flake* head, flake* node) {
	/* if first in list */
	if(node == head){
		head = remove_front(head);
		return head;
	}

	/* if last in list */
	if(node->next == NULL) {
		head = remove_back(head);
		return head;
	}

	/* if in the middle */
	flake* cursor = head;
	while(cursor != NULL) {
		if(cursor->next == node) {
			break;
		}
		cursor = cursor->next;
	}

	if(cursor != NULL) {
		flake* tmp = cursor->next;
		cursor->next = tmp->next;
		tmp->next = NULL;
		free(tmp);
	}

	return head;
}

void cleanup(flake *head) {
	flake *cursor, *tmp;

	if(head != NULL) {
		cursor = head->next;
		head->next = NULL;
		while(cursor != NULL) {
			tmp = cursor->next;
			free(cursor);
			cursor = tmp;
		}
	}
}

void traverse(flake* head, callback f) {
	flake* cursor = head;
	while(cursor != NULL) {
		f(cursor);
		cursor = cursor->next;
	}
}

flake* remove_dead(flake* head) {
	flake* cursor = head;
	flake* tmp = NULL;
	while(cursor != NULL) {
		if(cursor->age <= 0) {
			tmp = cursor->next;
			head = remove_any(head, cursor);
			cursor = tmp;
		} else {
			cursor = cursor->next;
		}
	}

	return head;
}

int count(flake* head) {
	flake* cursor = head;
	int count = 0;
	while(cursor != NULL) {
		count++;
		cursor = cursor->next;
	}
	return count;
}

void draw_flake(flake* fl) {
	if(fl != NULL)
		mvaddwstr(fl->y, fl->x, sprite[fl->spriteindex]);
		 
}

void update_flake(flake* fl) {
	if(fl != NULL) {
		/* move the flake */
		if(fl->age > 0) {
			fl->age = fl->age - fl->speed;
			fl->y = fl->y + fl->speed;
		}
	}
}


int main(void)
{
	int maxy;
	int maxx;

	int flake_count = 0;
	int flakes_to_make = 0;
	flake* head = NULL;
	callback draw = draw_flake;
	callback update = update_flake;

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
	head = (flake*)malloc(sizeof(flake));
	if(head == NULL) return 1;
	head = create(0, 0, 0, maxy, 1, NULL);

	// Main animation loop that exits when ESC is pressed
	while(getch() != 27)
	{
		/* Update if screen size changes */
		getmaxyx(stdscr, maxy, maxx);

		/* Move existing flakes */
		traverse(head, update);
		head = remove_dead(head);
		flake_count = count(head);

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
				/* Get random speed */
				int speed = (rand() / (RAND_MAX / MAX_SPEED + 1) + 1);
				/* Add flake to the list */
				head = append(head, randx, 0, spritenum, maxy, speed);
			}
		}

		clear();
		traverse(head, draw);
		refresh();
	}

	cleanup(head); /* Cleanup particle system */
	endwin(); /* Cleanup curses */

	return 0;
}

