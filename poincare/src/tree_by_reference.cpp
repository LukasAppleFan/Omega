#include <poincare/tree_by_reference.h>
#include <poincare/ghost_reference.h>
#if POINCARE_TREE_LOG
#include <iostream>
#endif

namespace Poincare {

/* Constructors */

/* Clone */

TreeByReference TreeByReference::clone() const {
  if (isStatic()) {
    // Static nodes are not copied
    return TreeByReference(node());
  }
  /* TODO Remove ? if (isUninitialized()) {
    return TreeByReference();
  }*/
  TreeNode * myNode = node();
  if (myNode->isAllocationFailure()) {
    int allocationFailureNodeId = myNode->allocationFailureNodeIdentifier();
    return TreeByReference(TreePool::sharedPool()->node(allocationFailureNodeId));
  }
  TreeNode * nodeCopy = TreePool::sharedPool()->deepCopy(myNode);
  return TreeByReference(nodeCopy);
}

/* Hierarchy operations */

void TreeByReference::replaceWithInPlace(TreeByReference t) {
  assert(!isUninitialized());
  if (isAllocationFailure()) {
    return;
  }
  TreeByReference p = parent();
  if (!p.isUninitialized()) {
    p.replaceChildInPlace(*this, t);
  }
}

void TreeByReference::replaceChildInPlace(TreeByReference oldChild, TreeByReference newChild) {
  assert(!oldChild.isUninitialized());
  assert(!newChild.isUninitialized());

  if (oldChild == newChild) {
    return;
  }

  assert(!isUninitialized());
  if (isAllocationFailure()) {
    return;
  }
  if (newChild.isAllocationFailure()) {
    replaceWithAllocationFailureInPlace(numberOfChildren());
    return;
  }

  // If the new node is static, copy it in the pool and add the copy
  if (newChild.isStatic()) {
    TreeByReference newT = TreeByReference(TreePool::sharedPool()->deepCopy(newChild.node()));
    if (newT.isAllocationFailure()) {
      replaceWithAllocationFailureInPlace(numberOfChildren());
      return;
    }
    replaceChildInPlace(oldChild, newT);
    return;
  }

  // If the new child has a parent, detach from it
  newChild.detachFromParent();

  // Move the new child
  assert(newChild.isGhost() || newChild.parent().isUninitialized());
  TreePool::sharedPool()->move(oldChild.node(), newChild.node(), newChild.numberOfChildren());
  /* We could have moved the new node to oldChild.node()->nextSibling(), but
   * nextSibling is not computed correctly if we inserted an
   * AllocationFailureNode next to newChild. */
  newChild.node()->retain();

  // Move the old child
  TreePool::sharedPool()->move(TreePool::sharedPool()->last(), oldChild.node(), oldChild.numberOfChildren());
  oldChild.node()->release(oldChild.numberOfChildren());
}

void TreeByReference::replaceChildAtIndexInPlace(int oldChildIndex, TreeByReference newChild) {
  if (oldChildIndex < 0 || oldChildIndex >= numberOfChildren()) {
    /* The only case where the index might be out of range is when a tree has
     * become an allocation failure, in which case we need to escape the invalid
     * child removal. */
    assert(isAllocationFailure());
    return;
  }
  TreeByReference oldChild = childAtIndex(oldChildIndex);
  replaceChildInPlace(oldChild, newChild);
}

void TreeByReference::replaceWithAllocationFailureInPlace(int currentNumberOfChildren) {
  if (isAllocationFailure()) {
    return;
  }
  assert(!isUninitialized());
  TreeByReference p = parent();
  bool hasParent = !p.isUninitialized();
  int indexInParentNode = hasParent ? node()->indexInParent() : -1;
  int currentRetainCount = node()->retainCount();
  TreeNode * staticAllocFailNode = node()->failedAllocationStaticNode();

  // Release all children and delete the node in the pool
  TreePool::sharedPool()->removeChildrenAndDestroy(node(), currentNumberOfChildren);
  /* WARNING: If we called "p.decrementNumberOfChildren()" here, the number of
   * children of the parent layout would be:
   * -> numberOfChildren() for "dynamic trees" that have a m_numberOfChildren
   * variable (such as HorizontalLayout)
   * -> numberOfChildren() - 1 for "static trees" that have a fixed number of
   * children (such as IntegralLayout)
   *
   * By not decrementing the parent's number of children here, we now that it
   * has (numberOfChildren() - 1) children. */

  /* Create an allocation failure node with the previous node id. We know
   * there is room in the pool as we deleted the previous node and an
   * AllocationFailure nodes size is smaller or equal to any other node size.*/
  //TODO static assert that the size is smaller
  TreeNode * newAllocationFailureNode = TreePool::sharedPool()->deepCopy(staticAllocFailNode);
  newAllocationFailureNode->rename(m_identifier, true);
  newAllocationFailureNode->retain();
  if (hasParent) {
    assert(indexInParentNode >= 0);
    /* Set the refCount to previousRefCount-1 because the previous parent is
     * no longer retaining the node. When we add this node to the parent, it
     * will retain it and increment the retain count. */
    newAllocationFailureNode->setReferenceCounter(currentRetainCount - 1);
    p.addChildAtIndexInPlace(TreeByReference(newAllocationFailureNode), indexInParentNode, p.numberOfChildren() - 1);
    p.decrementNumberOfChildren();
    /* We decrement here the parent's number of children, as we did not do it
     * before, see WARNING. */
  } else {
    newAllocationFailureNode->setReferenceCounter(currentRetainCount);
  }
}

void TreeByReference::replaceChildWithGhostInPlace(TreeByReference t) {
  GhostReference ghost;
  return replaceChildInPlace(t, ghost);
}

void TreeByReference::mergeChildrenAtIndexInPlace(TreeByReference t, int i) {
  /* mergeChildrenAtIndexInPlace should only be called with a tree thant can
   * have any number of children, so there is no need to replace the stolen
   * children with ghosts. */
  // TODO assert this and t are "dynamic" trees
  assert(i >= 0 && i <= numberOfChildren());
  // Steal operands
  int numberOfNewChildren = t.numberOfChildren();
  if (i < numberOfChildren()) {
    TreePool::sharedPool()->moveChildren(node()->childAtIndex(i), t.node());
  } else {
    TreePool::sharedPool()->moveChildren(node()->lastDescendant()->next(), t.node());
  }
  node()->incrementNumberOfChildren(numberOfNewChildren);
  t.node()->eraseNumberOfChildren();
  // If t is a child, remove it
  if (node()->hasChild(t.node())) {
    removeChildInPlace(t, 0);
  }
}

void TreeByReference::swapChildrenInPlace(int i, int j) {
  assert(!isStatic());
  assert(i >= 0 && i < numberOfChildren());
  assert(j >= 0 && j < numberOfChildren());
  if (i == j) {
    return;
  }
  int firstChildIndex = i < j ? i : j;
  int secondChildIndex = i > j ? i : j;
  TreeByReference firstChild = childAtIndex(firstChildIndex);
  TreeByReference secondChild = childAtIndex(secondChildIndex);
  TreePool::sharedPool()->move(firstChild.node()->nextSibling(), secondChild.node(), secondChild.numberOfChildren());
  TreePool::sharedPool()->move(childAtIndex(secondChildIndex).node()->nextSibling(), firstChild.node(), firstChild.numberOfChildren());
}

#if POINCARE_TREE_LOG
void TreeByReference::log() const {
  node()->log(std::cout);
  std::cout << std::endl;
}
#endif

/* Protected */

// Add
void TreeByReference::addChildAtIndexInPlace(TreeByReference t, int index, int currentNumberOfChildren) {
  assert(!isUninitialized());
  assert(!t.isUninitialized());

  if (isAllocationFailure()) {
    return;
  }
  if (t.isAllocationFailure()) {
    replaceWithAllocationFailureInPlace(currentNumberOfChildren);
    return;
  }

  assert(index >= 0 && index <= currentNumberOfChildren);

  // If the new node is static, copy it in the pool and add the copy
  if (t.isStatic()) {
    TreeByReference newT = TreeByReference(TreePool::sharedPool()->deepCopy(t.node()));
    if (newT.isAllocationFailure()) {
      replaceWithAllocationFailureInPlace(currentNumberOfChildren);
      return;
    }
    addChildAtIndexInPlace(newT, index, currentNumberOfChildren);
    return;
  }

  // If t has a parent, detach t from it.
  t.detachFromParent();
  assert(t.parent().isUninitialized());

  // Move t
  TreeNode * newChildPosition = node()->next();
  for (int i = 0; i < index; i++) {
    newChildPosition = newChildPosition->nextSibling();
  }
  TreePool::sharedPool()->move(newChildPosition, t.node(), t.numberOfChildren());
  t.node()->retain();
  node()->incrementNumberOfChildren();

  node()->didAddChildAtIndex(currentNumberOfChildren+1);
}

// Remove

void TreeByReference::removeChildAtIndexInPlace(int i) {
  assert(!isUninitialized());
  if (i < 0 || i >= numberOfChildren()) {
    /* The only case where the index might be out of range is when a tree has
     * become an allocation failure, in which case we need to escape the invalid
     * child removal. */
    assert(isAllocationFailure());
    return;
  }
  TreeByReference t = childAtIndex(i);
  removeChildInPlace(t, t.numberOfChildren());
}

void TreeByReference::removeChildInPlace(TreeByReference t, int childNumberOfChildren) {
  assert(!isUninitialized());
  TreePool::sharedPool()->move(TreePool::sharedPool()->last(), t.node(), childNumberOfChildren);
  t.node()->release(childNumberOfChildren);
  node()->decrementNumberOfChildren();
}

void TreeByReference::removeChildrenInPlace(int currentNumberOfChildren) {
  assert(!isUninitialized());
  TreePool::sharedPool()->removeChildren(node(), currentNumberOfChildren);
}

/* Private */

void TreeByReference::detachFromParent() {
  TreeByReference myParent = parent();
  if (!myParent.isUninitialized()) {
    int idxInParent = myParent.indexOfChild(*this);
    myParent.childAtIndexWillBeStolen(idxInParent);
  }
  assert(parent().isUninitialized());
}

void TreeByReference::setTo(const TreeByReference & tr) {
  /* We cannot use (*this)==tr because tr would need to be casted to
   * TreeByReference, which calls setTo and triggers an infinite loop */
  if (identifier() == tr.identifier()) {
    return;
  }
  TreeNode * currentNode = node();
  setIdentifierAndRetain(tr.identifier());
  currentNode->release(currentNode->numberOfChildren());
}

}
