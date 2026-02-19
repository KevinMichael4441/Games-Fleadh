#ifndef EVENTS_H
#define EVENTS_H

typedef enum Event
{
	EVENT_NONE,
	EVENT_MOVE,
	EVENT_JUMP,
	EVENT_COLLIDE_UP,
	EVENT_COLLIDE_DOWN,
	EVENT_COLLIDE_HORIZONTAL, 
	EVENT_TIMER			// For events that happen after a timer (Resolution of Collision)
	//EVENT_INSTANT		// For events that happen instantly after another. EX: 'JUMPING' is an instant state
} Event;

#endif