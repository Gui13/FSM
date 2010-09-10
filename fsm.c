/*
 * fsm.c
 *
 *  Created on: 3 août 2010
 *      Author: gui13
 */

#include <stdlib.h>
#include <stdio.h>

#include "fsm.h"

#define FSM_MARKER 0xD1CEA5ED

struct _fsm_transition_table
{
	int marker; /* identifier to make sure that the FSM structure which is passed is a correct FSM */
	fsm_transition_pf **fsm_table;
	unsigned int current_state;
	unsigned int fsm_max_event;
	unsigned int fsm_max_state;

	void *pv_data;
};


int fsm_generate( fsm_transition *transitions, int transition_nb, unsigned int start_state, void *pv_data, fsm_transition_table **transition_table )
{
	/* Checks */
	if(transitions && transition_nb && transition_table)
	{

		/* First step: determine the transition table size. We simply go through the FSM transitions to get the max values of state and events */
		unsigned int max_state = 0;
		unsigned int max_event = 0;
		int i;

		for(i=0; i< transition_nb; i++)
		{
			if (transitions[i].fsm_state > max_state)
				max_state = transitions[i].fsm_state;
			if (transitions[i].fsm_event > max_event)
				max_event = transitions[i].fsm_event;
			printf("Transition: st %d : e %d -> cb %x\n",transitions[i].fsm_state,transitions[i].fsm_event, (unsigned int)transitions[i].fsm_cb);
		}
		printf("Max state: %d max event: %d\n", max_state+1, max_event+1);
		if(max_state && max_event && (start_state < max_state))
		{
			/* Now that we know about the size, allocate the fsm_transition_table */
			fsm_transition_table *table = calloc(1, sizeof(fsm_transition_table));
			if(table)
			{
				table->marker 		 = FSM_MARKER;
				table->fsm_max_event = max_event+1;
				table->fsm_max_state = max_state+1;
				table->current_state = start_state;
				table->pv_data       = pv_data;

				table->fsm_table 	 = calloc( 1, table->fsm_max_event * sizeof(fsm_transition_pf *) );
				for(i=0; i<table->fsm_max_event; i++)
				{

					table->fsm_table[i] = calloc( 1, table->fsm_max_state * sizeof(fsm_transition_pf) );
					if(!table->fsm_table[i])
						/* todo: free the allocated memory!! */
						return FSM_ERR_NO_MEMORY;
				}

				if(table->fsm_table)
				{
					/* table is alloc'ed, now fill it in */
					int k=0;
					for(k=0; k< transition_nb; k++)
					{
						fsm_transition *t = &(transitions[k]);
						printf("Adding transition: st %d : e %d -> cb %x\n",t->fsm_state,t->fsm_event, (unsigned int)t->fsm_cb);
						if(table->fsm_table[t->fsm_event][t->fsm_state] != NULL)
							printf("Warning: FSM Overwriting callback for state %d and event %d\n", t->fsm_state, t->fsm_event);
						table->fsm_table[t->fsm_event][t->fsm_state] = t->fsm_cb;
					}
					*transition_table = table;
					return FSM_NO_ERROR;
				}
			}
			else
			{
				return FSM_ERR_NO_MEMORY;
			}

		}
		else
		{
			return FSM_ERR_ARGUMENT_ERROR;
		}

		return FSM_NO_ERROR;
	}
	else
	{
		/* One of the arguments is bad */
		return FSM_ERR_ARGUMENT_ERROR;
	}
}

int fsm_handle_event( fsm_transition_table *t, unsigned int event)
{
    /* Check the marker and that the event corresponds to a valid event */
	if( t && (t->marker == FSM_MARKER)
		  && (event < t->fsm_max_event))
	{
		fsm_transition_pf transition = t->fsm_table[event][t->current_state];
		printf("Handling event %d (current state: %d)\n", event, t->current_state);
		if (transition)
		{
            /* There's a transition, handle it */
			t->current_state = transition(event, t->pv_data);
			return FSM_NO_ERROR;
		}
        /* No transition for this event, notify caller */
		return FSM_ERR_NO_TRANSITION;
	}
	else
	{
        /* Bad argument.. */
		return FSM_ERR_ARGUMENT_ERROR;
	}
}

int fsm_free( fsm_transition_table *table)
{
	if(table && table->marker == FSM_MARKER)
	{
		int i;
		for(i=0; i<table->fsm_max_event; i++)
			if(table->fsm_table[i])
				free(table->fsm_table[i]);
		free(table->fsm_table);
		free(table);
		return FSM_NO_ERROR;
	}
	else
	{
		return FSM_ERR_ARGUMENT_ERROR;
	}
}

int fsm_get_current_state( fsm_transition_table *t)
{
    if(t && t->marker == FSM_MARKER )
        return t->current_state;
    else {
        return FSM_ERR_ARGUMENT_ERROR;
    }

}
