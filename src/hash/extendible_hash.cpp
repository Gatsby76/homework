#include <list>
#include<vector>

#include "hash/extendible_hash.h"
#include "page/page.h"

using namespace std;

namespace scudb {

  /*
  * constructor
  * array_size: fixed array size for each bucket
  */
  template <typename K, typename V>
  ExtendibleHash<K, V>::ExtendibleHash(size_t size) {
    this->GlobalDepth = 1;
    this->NumBuckets = 2;
    this->size = size;
    this->bucket.resize(2);
    this->find_bucket.resize(2);
    this->bucket[0].depth = 1;
    this->bucket[1].depth = 1;
    this->find_bucket[0] = 0;
    this->find_bucket[1] = 1;
  }

  /*
  * helper function to calculate the hashing address of input key
  */
  template <typename K, typename V>
  size_t ExtendibleHash<K, V>::HashKey(const K &key) {
    return find_bucket[hash<K>{}(key) % (1<<GlobalDepth)];
  }

  /*
  * helper function to return global depth of hash table
  * NOTE: you must implement this function in order to pass test
  */
  template <typename K, typename V>
  int ExtendibleHash<K, V>::GetGlobalDepth() const {
    return GlobalDepth;
  }

  /*
  * helper function to return local depth of one specific bucket
  * NOTE: you must implement this function in order to pass test
  */
  template <typename K, typename V>
  int ExtendibleHash<K, V>::GetLocalDepth(int bucket_id) const {
    return bucket[bucket_id].depth;
  }

  /*
  * helper function to return current number of bucket in hash table
  */
  template <typename K, typename V>
  int ExtendibleHash<K, V>::GetNumBuckets() const {
    return NumBuckets;
  }

  /*
  * lookup function to find value associate with input key
  */
  template <typename K, typename V>
  bool ExtendibleHash<K, V>::Find(const K &key, V &value) {
      std::lock_guard<std::mutex> guard(mutex);
    for(size_t i=0 ; i<bucket[HashKey(key)].content.size(); i++){
      if(key == bucket[HashKey(key)].content[i].key){
        value = bucket[HashKey(key)].content[i].value;
        return true;
      }  
    }  
    return false;
  }

  /*
  * delete <key,value> entry in hash table
  * Shrink & Combination is not required for this project
  */
  template <typename K, typename V>
  bool ExtendibleHash<K, V>::Remove(const K &key) {
      std::lock_guard<std::mutex> guard(mutex);
    bool exist = 0;
    for(size_t i=0 ; i<bucket[HashKey(key)].content.size(); i++){
      if(key == bucket[HashKey(key)].content[i].key){
        bucket[HashKey(key)].content.erase(bucket[HashKey(key)].content.begin()+i);
        exist = 1;
      }
    }  
    return exist;
  }

  /*
  * insert <key,value> entry in hash table
  * Split & Redistribute bucket when there is overflow and if necessary increase
  * global depth
  */
  template <typename K, typename V>
  void ExtendibleHash<K, V>::Insert(const K &key, const V &value) {
    std::lock_guard<std::mutex> guard(mutex);
    kv new_kv;
    new_kv.key = key;
    new_kv.value = value;
    while(bucket[HashKey(key)].content.size() == size){
        // 若桶的深度小于 Globaldepth， 就直接将 find_bucket 数组
      if(bucket[HashKey(key)].depth < GlobalDepth){
          //idx 是分裂后更高序号的桶的序号
        size_t ori_idx = HashKey(key);
        size_t idx = (1<<bucket[HashKey(key)].depth) + HashKey(key);
        bucket[idx].depth = bucket[HashKey(key)].depth + 1;
        bucket[HashKey(key)].depth++;
        find_bucket[idx] = idx;
        for(int i=bucket[idx].depth ; i<GlobalDepth; i++){
          find_bucket[idx+(1<<i)] = idx;
        }
        for(size_t i=0;i<bucket[ori_idx].content.size();i++){
          if(HashKey(bucket[ori_idx].content[i].key) == idx){
            bucket[idx].content.push_back(bucket[ori_idx].content[i]);
            bucket[ori_idx].content.erase(bucket[ori_idx].content.begin()+i);
            i--;
          }
        }
      }
        //若桶的深度等于globaldepth，桶分裂，
      if(bucket[HashKey(key)].depth == GlobalDepth && bucket[HashKey(key)].content.size() == size){
        GlobalDepth++;
        bucket.resize(1<<GlobalDepth);
        find_bucket.resize(1<<GlobalDepth);
        for(int i=(1<<(GlobalDepth-1)); i<(1<<GlobalDepth);i++){
            find_bucket[i] = find_bucket[i-(1<<(GlobalDepth-1))];
        }
        NumBuckets *= 2;
      }
    }
    bucket[HashKey(key)].content.push_back(new_kv);
  }

template class ExtendibleHash<page_id_t, Page *>;
template class ExtendibleHash<Page *, std::list<Page *>::iterator>;
// test purpose
template class ExtendibleHash<int, std::string>;
template class ExtendibleHash<int, std::list<int>::iterator>;
template class ExtendibleHash<int, int>;
} // namespace scudb
