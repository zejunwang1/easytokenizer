/**
 * Copyright (c) 2022-present, Zejun Wang (wangzejunscut@126.com).
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DTRIE_H
#define DTRIE_H

#include <fstream>
#include <cassert>
#include <cctype>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include "cedar.h"

namespace cedar {

typedef da<int> dar;

class DTrie {
  protected:
    size_t _size;
    size_t _max_word_len;
    
    std::unique_ptr<dar> _da;
    std::vector<std::string> _keys;
    
    int build(const std::string& dict_path, const std::string& user_dict_path) {
      std::ifstream in(dict_path);
      if (!in.is_open()) {
        throw std::invalid_argument(dict_path + " can not be opened for loading.");
      }

      std::string word;
      while (std::getline(in, word)) {
        if (word.empty()) continue;
        _keys.emplace_back(word);
        _max_word_len = (_max_word_len > word.length()) ? _max_word_len : word.length();
      }
      if (!user_dict_path.empty()) {
        std::ifstream inuser(user_dict_path);
        if (!inuser.is_open()) {
          throw std::invalid_argument(user_dict_path + " can not be opened for loading.");
        }
        while (std::getline(inuser, word)) {
          if (word.empty()) continue;
          _keys.emplace_back(word);
          _max_word_len = (_max_word_len > word.length()) ? _max_word_len : word.length();
        }
      }

      std::sort(_keys.begin(), _keys.end());
      _keys.erase(std::unique(_keys.begin(), _keys.end()), _keys.end());
      _size = _keys.size();
      std::vector<const char*> keys;
      for (size_t i = 0; i < _keys.size(); i++) {
        keys.emplace_back(_keys[i].c_str());
      }
      
      _da = std::unique_ptr<dar>(new dar());
      return _da->build(_size, &(keys[0]), 0, 0);
    }
  
  public:
    DTrie() : _size(0), _max_word_len(0) {
      _da = std::unique_ptr<dar>(new dar());
    }
    
    DTrie(const std::string& dict_path, const std::string& user_dict_path = "")
    : _size(0), _max_word_len(0) {
      if (build(dict_path, user_dict_path) != 0) {
        throw std::invalid_argument("build double-array-trie failed.");
      }
    }
    
    size_t size() { return _size; }
    
    size_t maxlen() { return _max_word_len; }
    
    void insert(const std::vector<std::string>& words) {
      for (size_t i = 0; i < words.size(); i++) {
        insert(words[i]);
      }
    }
    
    void insert(const std::string& word) {
      insert(word.c_str());
    }  

    void insert(const char* word) {
      insert(word, std::strlen(word));
    }
    
    void insert(const char* word, size_t len) {
      auto result_pair = _da->exactMatchSearch<dar::result_pair_type>(word, len);
      if (result_pair.value < 0) {
        _da->update(word, len, _size++);
        _keys.emplace_back(std::string(word, len));
        _max_word_len = (_max_word_len > len) ? _max_word_len : len;
      }
    }
    
    int index(const std::string& word) {
      auto result_pair = _da->exactMatchSearch<dar::result_pair_type>(word.c_str());
      return result_pair.value;
    }
   
    bool count(const std::string& word) {
      if (index(word) < 0)  { return false; }
      return true; 
    }
    
    std::string keys(int id) {
      assert(id >= 0);
      assert(id < _size);
      return _keys[id];
    }

    std::vector<std::pair<size_t, std::string>> parse(const std::string& text, size_t max_prefix_matches = 512) {
      const char* str = text.c_str();
      size_t bpos = 0, tlen = std::strlen(str);
      std::vector<std::pair<size_t, std::string>> matches;
      std::vector<dar::result_pair_type> result_pairs(max_prefix_matches);
      while (bpos < tlen) {
        size_t num = _da->commonPrefixSearch(str + bpos, &(result_pairs[0]), max_prefix_matches, tlen - bpos);
        for (size_t i = 0; i < num && i < result_pairs.size(); i++) {
          const dar::result_pair_type& result_pair = result_pairs[i];
          matches.emplace_back(bpos, _keys[result_pair.value]);
        }
        if (isascii(str[bpos])) { bpos++; } 
        else {
          bpos++;
          while (bpos < tlen && (str[bpos] & 0xC0) == 0x80) {
            bpos++;
          }
        }
      }
      return matches;
    }
    
    std::string max_prefix(const std::string& text, size_t max_prefix_matches = 512) {
      std::string res;
      std::vector<dar::result_pair_type> result_pairs(max_prefix_matches);
      size_t num = _da->commonPrefixSearch(text.c_str(), &(result_pairs[0]), max_prefix_matches);
      num = (num < result_pairs.size()) ? num : result_pairs.size();
      if (num < 1)  return res;
      const dar::result_pair_type& result_pair = result_pairs[num - 1];
      return _keys[result_pair.value];
    }
};

}
#endif
