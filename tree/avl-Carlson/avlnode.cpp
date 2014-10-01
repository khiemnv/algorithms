/* Filename:  avlnode.cpp

   Programmer:  Br. David Carlson

   Reference:  Data Structures with C++, by Ford and Topp

   Date:  October 12, 1997

   This file defines the constructor for the class AVLNodeClass, which
   is set up in avlnode.h.
*/

#include "avlnode.h"


/* Given:  Item          Data item to place in Info field.
           LeftPtr       Pointer to place in Left field.
           RightPtr      Pointer to place in Right field.
           BalanceValue  Value to place in Balance field.
   Task:   This is the constructor.  It's job is to create a new
           object containing the above 4 values.
   Return: Nothing directly, but the implicit object is created.
*/
AVLNodeClass::AVLNodeClass(const ItemType & Item,
   AVLNodeClass * LeftPtr, AVLNodeClass * RightPtr, int BalanceValue):
   // call BSTNodeClass constructor, initialize field:
   BSTNodeClass(Item, LeftPtr, RightPtr), Balance(BalanceValue)
   {
   #ifdef DEBUG
      cout << "DEBUG: AVLNodeClass constructor called" << endl;
   #endif
   }


