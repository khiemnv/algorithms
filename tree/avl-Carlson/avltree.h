/* Filename:  avltree.h

   Programmer:  Br. David Carlson

   Reference:  Data Structures with C++, by Ford and Topp.

   Date:  October 12, 1997

   Revised:  June 9, 1999 to use the type AVLNodePtr.

   This is the header file to accompany avltree.cpp.  It sets up the class
   AVLClass as shown below.  Note that the following functions are not
   overwritten, so that in any reference to them, these mean functions of
   the BSTClass:  SubtreeFind, PrintSubtree, NumItems, Empty, Find, Print.
*/

#ifndef AVLTREE_H
#define AVLTREE_H

#include "avlnode.h"
#include "bstree.h"


const int LeftHeavy = -1;
const int Balanced = 0;
const int RightHeavy = 1;


class AVLClass: public BSTClass
   {
   private:
      AVLNodePtr GetNode(const ItemType & Item,
         AVLNodePtr LeftPtr = NULL, AVLNodePtr RightPtr = NULL,
         int BalanceValue = 0);
      void CopyTree(const AVLClass & Tree);
      AVLNodePtr CopySubtree(const AVLNodePtr Current);
      void SingleRotateLeft(AVLNodePtr & ParentPtr);
      void SingleRotateRight(AVLNodePtr & ParentPtr);
      void DoubleRotateLeft(AVLNodePtr & ParentPtr);
      void DoubleRotateRight(AVLNodePtr & ParentPtr);
      void UpdateLeftTree(AVLNodePtr & ParentPtr, bool & ReviseBalance);
      void UpdateRightTree(AVLNodePtr & ParentPtr, bool & ReviseBalance);
      void AVLInsert(AVLNodePtr & Tree, AVLNodePtr NewNodePtr,
         bool & ReviseBalance);
      void FreeNode(AVLNodePtr NodePtr);
      void ClearTree(void);
      void ClearSubtree(AVLNodePtr Current);
   public:
      AVLClass(void);
      AVLClass(const AVLClass & Tree);
      ~AVLClass(void);
      AVLClass & operator= (const AVLClass & Tree);
      void Insert(const ItemType & Item);
      //  Some sort of Remove method could also be added, but it would
      //  take effort to remake the AVL tree after the deletion of a node.
   };

#endif


