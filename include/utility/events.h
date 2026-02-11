#ifndef EVENTS_H
#define EVENTS_H

typedef enum Event
{
	EVENT_NONE,
	EVENT_MOVE,
	EVENT_JUMP, 
	EVENT_INSTANT		// For events that happen instantly after another. EX: 'JUMPING' is an instant state
} Event;

#endif