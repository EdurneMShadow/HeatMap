/*
 * linkedList.cpp
 *
 *  Created on: 19/2/2016
 *      Author: edurne
 */

#include "linkedList.h"

#include <stddef.h>
#include <iostream>

linkedList::linkedList() {
	head = NULL;
}

void linkedList::addValue(int x, int y){
        Node *n = new Node();   // create new Node
        n->x = x;             // set value
        n->y = y;
        n->next = head;         // make the node point to the next node.
                                //  If the list is empty, this is NULL, so the end of the list --> OK
        head = n;               // last but not least, make the head point at the new node.
    }

void linkedList::siguienteNodo(){
	head=head->next;
	cout<<"Pasando al siguiente..."<<endl;
}
bool linkedList::vacia(){
	if(head!=NULL)
		return false;
	else return true;
}
int linkedList::devolverHead(int &y){
	int x = head->x;
	y=head->y;
	head=head->next;
	return x;
}
int linkedList::popValueX(){
        Node *n = head;
        int ret = n->x;

        head = head->next;
        delete n;
        return ret;
    }

int linkedList::popValueY(){
        Node *n = head;
        int ret = n->y;

        head = head->next;
        delete n;
        return ret;
    }

linkedList::~linkedList() {
	// TODO Auto-generated destructor stub
}
