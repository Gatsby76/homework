/**
 * LRU implementation
 */
#include "buffer/lru_replacer.h"
#include "page/page.h"

namespace scudb {

template <typename T> LRUReplacer<T>::LRUReplacer() {
  headNode = make_shared<Node>();
  tailNode = make_shared<Node>();
  headNode->nextNode = tailNode;
  tailNode->prevNode = headNode;
}

template <typename T> LRUReplacer<T>::~LRUReplacer() {}

/*
 * Insert value into LRU
 */
template <typename T> void LRUReplacer<T>::Insert(const T &value) {
  lock_guard<mutex> lock(latch);
  shared_ptr<Node> currNode;
  if (nodeMap.find(value) != nodeMap.end()) {
    currNode = nodeMap[value];
    shared_ptr<Node> prevNode = currNode->prevNode;
    shared_ptr<Node> nextNode = currNode->nextNode;
    prevNode->nextNode = nextNode;
    nextNode->prevNode = prevNode;
  } else {
    currNode = make_shared<Node>(value);
  }
  shared_ptr<Node> lastNode = tailNode->prevNode;
  currNode->prevNode = lastNode;
  currNode->nextNode = tailNode;
  lastNode->nextNode = currNode;
  tailNode->prevNode = currNode;
  nodeMap[value] = currNode;
}

/* If LRU is non-empty, pop the head member from LRU to argument "value", and
 * return true. If LRU is empty, return false
 */
template <typename T> bool LRUReplacer<T>::Victim(T &value) {
  lock_guard<mutex> lock(latch);
  if (nodeMap.empty()) {
    return false;
  }
  shared_ptr<Node> frstNode = headNode->nextNode;
  headNode->nextNode = frstNode->nextNode;
  frstNode->nextNode->prevNode = headNode;
  value = frstNode->value;
  nodeMap.erase(frstNode->value);
  return true;
}

/*
 * Remove value from LRU. If removal is successful, return true, otherwise
 * return false
 */
template <typename T> bool LRUReplacer<T>::Erase(const T &value) {
  lock_guard<mutex> lock(latch);
  if (nodeMap.find(value) != nodeMap.end()) {
    shared_ptr<Node> currNode = nodeMap[value];
    currNode->prevNode->nextNode = currNode->nextNode;
    currNode->nextNode->prevNode = currNode->prevNode;
  }
  return nodeMap.erase(value);
}

template <typename T> size_t LRUReplacer<T>::Size() { 
  lock_guard<mutex> lock(latch);
  size_t size = nodeMap.size();
  return size; 
}

template class LRUReplacer<Page *>;
// test only
template class LRUReplacer<int>;

} // namespace scudb
