#ifndef BYTECODES_H_
#define BYTECODES_H_



/*
 * The bytecodes representing the various netcode instructions
 */
#define FUTURE 				0
#define HALT 				1
#define IF 					2
#define MODE 				3
#define CREATE 				4
#define DESTROY 			5
#define SEND				6
#define RECEIVE				7
#define SYNC				8
#define HANDLE				9
#define GOTO				10
#define WAIT				11
#define XSEND				12
#define BACKUP_SEND			13
#define BACKUP_RECEIVE		14
#define NOP					15
#define END_OF_PROGRAM		16
#define SET_COUNTER			17
#define ADD_TO_COUNTER		18
#define CLEAR_COUNTER		19
#define SUB_FROM_COUNTER	20

#endif /* BYTECODES_H_ */
