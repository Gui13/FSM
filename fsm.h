/*
 * fsm.h
 *
 *  Created on: 3 août 2010
 *      Author: gui13
 */

#ifndef FSM_H_
#define FSM_H_
/*
 The idea behind this API is to have a prettier and faster way to describe a FSM. 
 As of today, FSM basically consist
 - either of a double dimension table which size is (Nb of states) * (Nb of event).
 handler_cb table[i][k]={
            event1     event2      event3
 stateX	{   handler1,   NULL,       NULL},
 stateY	{   handler2,   handler3,   NULL}
 };
 This maximizes the handling time of a particular {event, state} sequence, since 
 accessing an element of this table is simply a matter of table[event][state].
 - or a table that describes the state machine as a list of event/state/handler:
 {STATE_X, EVENT1, handler_1},
 {STATE_Y, EVENT1, handler_2},
 {STATE_Y, EVENT2, handler_3}
 This method is prettier and easier to read, but slow to handle, since you have 
 to walk the table to handle an event.
 
 Here, I try to merge the two behaviors: 
    - you define a "pretty" table that describes your state machine,
    - you use this API to dynamically create the "fast version" of the table.
 
 You can then use this table to make your state machine run.
 */

/* errors you can get when using the API */
typedef enum {
	FSM_NO_ERROR            = -1, /* everything went fine */
    
	FSM_ERR_NO_TRANSITION   = -2, /* the {event,state} couple has no defined 
                                    transition */

	FSM_ERR_ARGUMENT_ERROR  = -3, /* one of the arguments you passed is wrong */
    
	FSM_ERR_NO_MEMORY       = -4  /* problem allocating the memory for the 
                                     transition table */
} fsm_error_t;


/* opaque structure used when generating/using/freeing a transition table */
typedef struct _fsm_transition_table fsm_transition_table;

/**
 * When the FSM receives a transition event, it calls a transition callback (if 
 * a callback is defined for this particular {state,event} tuple).
 *
 * This transition callback receives the event of the FSM and is given an eventual 
 * private data.
 *
 * The callback should return the new state in which the FSM should be placed.
 */
typedef int (*fsm_transition_pf)(int event, void *pv_data);


/* The structure of a "transition": the state your FSM is in, the event, and the 
 function that will handle this event. 
 NOTE:
 It is IMPORTANT that you make your states and events start from 0 and grow 
 incrementaly. Since the transition table is allocated following the max value 
 of your states and the max value of your events, using big numbers will get you
 a BIG FSM.
 */
typedef struct _fsm_transition
{
	unsigned int fsm_state;		/* state where the FSM is right now */
	unsigned int fsm_event;		/* event received by the FSM */
	fsm_transition_pf fsm_cb;	/* transition to apply to the FSM, which returns 
                                   the new FSM state */
} fsm_transition;


/**
 Use this function to generate a fsm_transition_table from a list of 
 fsm_transitions. 
 You can give a default "starting" state, as well as a private data that will be 
 passed on each transition.
 
 \param[in] transitions a pointer to the transitions you want to include in your FSM.
 \param[in] transition_nb the size of the table of transitions you pass
 \param[in] start_state the initial state in which the FSM is to be placed
 \param[in] transition_table the address of a pointer which will receive the 
            generated FSM table identifier.
 */
int fsm_generate( fsm_transition *transitions, 
                  int transition_nb, 
                  unsigned int start_state, 
                  void *pv_data, 
                  fsm_transition_table **transition_table );

/**
 Use this function to deallocate an allocated FSM.
 \param[in] table the allocated FSM.
 \return FSM_ERR_ARGUMENT_ERROR if the pointer is invalid, FSM_NO_ERROR is all 
         went fine.
 */
int fsm_free( fsm_transition_table *table);

/* Use the transition table t to handle the event you pass. 
 \param[in] t the transition table you generated 
 \param[in] event the event you want to handle.
 \return FSM_NO_ERROR if no error happened, FSM_ERR_NO_TRANSITION if no 
 transition exists for the current {state, event} couple, FSM_ERR_ARGUMENT_ERROR
 if one of the arguments is bad. */
int fsm_handle_event(fsm_transition_table *t, unsigned int event);

int fsm_get_current_state( fsm_transition_table *t);

#endif /* FSM_H_ */
