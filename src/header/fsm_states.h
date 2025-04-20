/**
 * Martin Bozko
 * xbozko01
 * 20.04.2025
 * 
 * enum for FSM states
 */
#ifndef FSM_STATES_H
#define FSM_STATES_H

typedef enum FSMState{
    START = 1,
    AUTH = 2,
    OPEN = 3,
    JOIN = 4,
    ENDING = 5,
    END = 0,
    BYE = 10
} FSMState;

#endif