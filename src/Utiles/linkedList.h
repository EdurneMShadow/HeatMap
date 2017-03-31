/*
 * linkedList.h
 *
 *  Created on: 19/2/2016
 *      Author: edurne
 */

#ifndef UTILES_LINKEDLIST_H_
#define UTILES_LINKEDLIST_H_

using namespace std;

class linkedList {
	// Struct inside the class LinkedList
	    // This is one node which is not needed by the caller. It is just
	    // for internal work.
	    struct Node {
	        int x;
	        int y;
	        Node *next;
	    };
	    Node *head;
public:
	linkedList();
	void addValue(int x, int y);
	int popValueX();
	int popValueY();
	int devolverHead(int &y);
	bool vacia();
	void siguienteNodo();
	virtual ~linkedList();
};

#endif /* UTILES_LINKEDLIST_H_ */
