/* Filename:  bstnode.h

   Programmer:  Br. David Carlson

   Date:  October 10, 1997

   Revised:  August 8, 1998

   Revised:  June 9, 1999 to use protected, not private, data fields.
   This is so that a derived class can directly access them.  An alternate
   solution would be to make any derived class (such as AVLNodeClass) a
   friend of this class.  This file was also revised to make AVLClass a
   friend of this class, so that it would have easy access to the data fields.

   This is the header file to accompany bstnode.cpp.  It provides the
   BSTNodeClass shown below as well as the BSTNodePtr type.
*/

#ifndef BSTNODE_H
#define BSTNODE_H

#include "itemtype.h"


class BSTNodeClass
   {
   protected:
      ItemType Info;
      BSTNodeClass * Left, * Right;
   public:
      BSTNodeClass(const ItemType & Item, BSTNodeClass * LeftPtr = NULL,
         BSTNodeClass * RightPtr = NULL):
         Info(Item), Left(LeftPtr), Right(RightPtr)
         {
         };
      void GetInfo(ItemType & TheInfo) const;
   friend class BSTClass;
   friend class AVLClass;
   };

typedef BSTNodeClass * BSTNodePtr;

#endif


