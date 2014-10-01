/* Filename:  avlnode.h

   Programmer:  Br. David Carlson

   Reference:  Data Structures with C++, by Ford and Topp

   Date:  October 12, 1997

   Modified:  June 9, 1999 to create AVLNodePtr type.

   This is the header file to accompany avlnode.cpp.  It sets up the
   class AVLNodeClass as shown below.
*/

#ifndef AVLNODE_H
#define AVLNODE_H

#include "bstnode.h"


class AVLNodeClass: public BSTNodeClass
   {
   private:
      int Balance;
   public:
      // constructor:
      AVLNodeClass(const ItemType & Item, AVLNodeClass * LeftPtr = NULL,
         AVLNodeClass * RightPtr = NULL, int BalanceValue = 0);
   friend class AVLClass;
   friend class AVLTableClass;   // may not get to implementing this
   };

typedef AVLNodeClass * AVLNodePtr;

#endif


