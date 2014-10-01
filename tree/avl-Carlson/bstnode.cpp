/* Filename:  bstnode.cpp

   Programmer:  Br. David Carlson

   Date:  October 10, 1997

   Modified:  August 8, 1998.

   This file implements the class functions for the BSTNodeClass, which
   is declared in bstnode.h.  Since no code is needed for the
   constructor, only the GetInfo function is found here.
*/

#include "bstnode.h"


/* Given:  Nothing other than the implicit object.
   Task:   To look up the Info field of the object.
   Return: A copy of this data in TheInfo.
*/
void BSTNodeClass::GetInfo(ItemType & TheInfo) const
   {
   TheInfo = Info;   // assumes assignment works on this type
   }


