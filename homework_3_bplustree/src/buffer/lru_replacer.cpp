/**
 * LRU implementation
 */
#include "buffer/lru_replacer.h"
#include "page/page.h"

namespace scudb {

template <typename T> 
LRUReplacer<T>::LRUReplacer() {
  this->size = 0;
}

template <typename T> 
LRUReplacer<T>::~LRUReplacer() {
  lru_map.clear();
}

/*
 * Insert value into LRU
 */
template <typename T> 
void LRUReplacer<T>::Insert(const T &value) {
  if(lru_map.find(value) == lru_map.end()){
    lru_map.insert(make_pair(value, 0));
    size++;
  }
  else 
    lru_map[value] = 0;
  for(iter = lru_map.begin(); iter != lru_map.end(); iter++){
    (iter->second)++;
  }
}

/* If LRU is non-empty, pop the head member from LRU to argument "value", and
 * return true. If LRU is empty, return false
 */
template <typename T> 
bool LRUReplacer<T>::Victim(T &value) {
  if(Size() == 0)
    return false;
  else {
    T key ; int max = -1;
    for(iter = lru_map.begin(); iter != lru_map.end(); iter++){
      if((iter->second) > max){
        max = iter->second;
        key = iter->first;
      }
    }
    value = key;
    lru_map.erase(key);
    size--;
    return true;
  }
}

/*
 * Remove value from LRU. If removal is successful, return true, otherwise
 * return false
 */
template <typename T> 
bool LRUReplacer<T>::Erase(const T &value) {
  if(lru_map.find(value) == lru_map.end())
    return false;
  else {
    lru_map.erase(value);
    size--;
    return true;
  }
}

template <typename T> 
size_t LRUReplacer<T>::Size() { 
  return size; 
}

template class LRUReplacer<Page *>;
// test only
template class LRUReplacer<int>;

} // namespace scudb
