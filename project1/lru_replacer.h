/**
 * lru_replacer.h
 *
 * Functionality: The buffer pool manager must maintain a LRU list to collect
 * all the pages that are unpinned and ready to be swapped. The simplest way to
 * implement LRU is a FIFO queue, but remember to dequeue or enqueue pages when
 * a page changes from unpinned to pinned, or vice-versa.
 */

#pragma once

#include "buffer/replacer.h"
#include <mutex>
#include <memory>
#include <map>

using namespace std;

namespace scudb {

template <typename T> class LRUReplacer : public Replacer<T> {
  struct Node {
    Node() {};
    Node(T value){
      this->value = value;
    };
    T value;
    shared_ptr<Node> prevNode;
    shared_ptr<Node> nextNode;
  };
public:
  // do not change public interface
  LRUReplacer();

  ~LRUReplacer();

  void Insert(const T &value);

  bool Victim(T &value);

  bool Erase(const T &value);

  size_t Size();

private:
  shared_ptr<Node> headNode;
  shared_ptr<Node> tailNode;
  map<T,shared_ptr<Node>> nodeMap;
  mutable mutex latch;
  // add your member variables here
};

} // namespace scudb
