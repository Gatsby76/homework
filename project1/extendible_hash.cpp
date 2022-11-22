#include <list>

#include "hash/extendible_hash.h"
#include "page/page.h"
using namespace std;

namespace scudb {

/*
 * constructor
 * array_size: fixed array size for each bucket
 */
template <typename K, typename V>
ExtendibleHash<K, V>::ExtendibleHash(size_t size){
  globalDepth = 0;
  bucketSize = size;
  bucketNum = 1;
  buckets.push_back(make_shared<Bucket>(0));
}
template<typename K, typename V>
ExtendibleHash<K, V>::ExtendibleHash() {
  ExtendibleHash(64);
}
/*
 * helper function to calculate the hashing address of input key
 */
template <typename K, typename V>
size_t ExtendibleHash<K, V>::HashKey(const K &key) const{
  size_t res = hash<K>{}(key);
  return res;
}

/*
 * helper function to return global depth of hash table
 * NOTE: you must implement this function in order to pass test
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetGlobalDepth() const {
  lock_guard<mutex> tableLock(tableLatch);
  return globalDepth;
}

/*
 * helper function to return local depth of one specific bucket
 * NOTE: you must implement this function in order to pass test
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetLocalDepth(int bucket_id) const {
  if (buckets[bucket_id]) {
    lock_guard<mutex> bucketLock(buckets[bucket_id]->bucketLatch);
    if (buckets[bucket_id]->itemMap.size() == 0) return -1;
    return buckets[bucket_id]->localDepth;
  }
  return -1;
}

/*
 * helper function to return current number of bucket in hash table
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetNumBuckets() const {
  lock_guard<mutex> tableLock(tableLatch);
  return bucketNum;
}

/*
 * lookup function to find value associate with input key
 */
template <typename K, typename V>
bool ExtendibleHash<K, V>::Find(const K &key, V &value) {
  int bucketIndex = GetBucketIndex(key);
  lock_guard<mutex> bucketLock(buckets[bucketIndex]->bucketLatch);
  if (buckets[bucketIndex]->itemMap.find(key) != buckets[bucketIndex]->itemMap.end()) {
    value = buckets[bucketIndex]->itemMap[key];
    return true;
  }
  return false;
}

template <typename K, typename V>
int ExtendibleHash<K, V>::GetBucketIndex(const K &key) const{
  lock_guard<mutex> tableLock(tableLatch);
  return HashKey(key) & ((1 << globalDepth) - 1);
}

/*
 * delete <key,value> entry in hash table
 * Shrink & Combination is not required for this project
 */
template <typename K, typename V>
bool ExtendibleHash<K, V>::Remove(const K &key) {
  int bucketIndex = GetBucketIndex(key);
  lock_guard<mutex> bucketLock(buckets[bucketIndex]->bucketLatch);
  shared_ptr<Bucket> curBucket = buckets[bucketIndex];
  if (curBucket->itemMap.find(key) != curBucket->itemMap.end()) {
    curBucket->itemMap.erase(key);
    return true;
  }
  return false;
}

/*
 * insert <key,value> entry in hash table
 * Split & Redistribute bucket when there is overflow and if necessary increase
 * global depth
 */
template <typename K, typename V>
void ExtendibleHash<K, V>::Insert(const K &key, const V &value) {
  int bucketIndex = GetBucketIndex(key);
  shared_ptr<Bucket> curBucket = buckets[bucketIndex];
  while (true) {
    lock_guard<mutex> bucketLock(curBucket->bucketLatch);
    if (curBucket->itemMap.find(key) != curBucket->itemMap.end() || curBucket->itemMap.size() < bucketSize) {
      curBucket->itemMap[key] = value;
      break;
    }
    int mask = (1 << (curBucket->localDepth));
    curBucket->localDepth++;

    {
      lock_guard<mutex> tableLock(tableLatch);
      if (curBucket->localDepth > globalDepth) {

        size_t length = buckets.size();
        for (size_t i = 0; i < length; i++) {
          buckets.push_back(buckets[i]);
        }
        globalDepth++;

      }
      bucketNum++;
      auto newBucket = make_shared<Bucket>(curBucket->localDepth);

      typename map<K, V>::iterator it;
      for (it = curBucket->itemMap.begin(); it != curBucket->itemMap.end(); it++) {
        if (HashKey(it->first) & mask) {
          newBucket->itemMap[it->first] = it->second;
          it = curBucket->itemMap.erase(it);
          it--;
        }
      }
      for (size_t i = 0; i < buckets.size(); i++) {
        if (buckets[i] == curBucket && (i & mask))
          buckets[i] = newBucket;
      }
    }
    bucketIndex = GetBucketIndex(key);
    curBucket = buckets[bucketIndex];
  }
}

template class ExtendibleHash<page_id_t, Page *>;
template class ExtendibleHash<Page *, std::list<Page *>::iterator>;
// test purpose
template class ExtendibleHash<int, std::string>;
template class ExtendibleHash<int, std::list<int>::iterator>;
template class ExtendibleHash<int, int>;
} // namespace scudb
