/*
 * extendible_hash.h : implementation of in-memory hash table using extendible
 * hashing
 *
 * Functionality: The buffer pool manager must maintain a page table to be able
 * to quickly map a PageId to its corresponding memory location; or alternately
 * report that the PageId does not match any currently-buffered page.
 */

#pragma once

#include <cstdlib>
#include <vector>
#include <string>
#include <mutex>

#include "hash/hash_table.h"
#include<vector>

using namespace std;

namespace scudb {

  template <typename K, typename V>
  class ExtendibleHash : public HashTable<K, V> {
    public:
      // constructor
      ExtendibleHash(size_t size);
      // helper function to generate hash addressing
      size_t HashKey(const K &key);
      // helper function to get global & local depth
      int GetGlobalDepth() const;
      int GetLocalDepth(int bucket_id) const;
      int GetNumBuckets() const;
      // lookup and modifier
      bool Find(const K &key, V &value) override;
      bool Remove(const K &key) override;
      void Insert(const K &key, const V &value) override;

    private:
      struct kv{
        K key;
        V value;
      };
      struct Bucket{
        int depth;
        vector<kv> content;
      };
      vector<Bucket> bucket;
      int GlobalDepth;
      size_t NumBuckets;
      size_t size;
      vector<int> find_bucket;
      std::mutex mutex;
      // add your own member variables here
  };
} // namespace scudb
