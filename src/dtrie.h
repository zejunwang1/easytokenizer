/**
 * Copyright (c) 2022-present, Zejun Wang (wangzejunscut@126.com)
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DTRIE_H
#define DTRIE_H

#include <cctype>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "cedar.h"

namespace cedar
{

typedef da<int> dar;

class DTrie
{
  public:
    using dar = da<int>;
    using result_type = dar::result_pair_type;
  
  private:
    int build(const std::string& vocab_path)
    {
      std::ifstream ifs(vocab_path);
      if (!ifs.is_open())
        throw std::invalid_argument(vocab_path + " can not be opened for loading!");

      std::string word;
      while (std::getline(ifs, word))
        if (!word.empty())
          _key.emplace_back(word);
      ifs.close();

      _size = _key.size();
      std::vector<size_t> len(_size);
      std::vector<const char*> key(_size);
      for (size_t i = 0; i < _size; i++)
      {
        len[i] = _key[i].size();
        key[i] = _key[i].data();
      }

      _da = std::unique_ptr<dar>(new dar());
      return _da->build(_size, key.data(), len.data());
    }

    public:
    DTrie() : _size(0)
    { _da = std::unique_ptr<dar>(new dar()); }

    DTrie(const std::string& vocab_path)
    {
      if (build(vocab_path))
        throw std::invalid_argument("build double-array trie failed!");
    }

    size_t size() const
    { return _size; }

    std::string get_key(size_t id) const
    {
      assert(id < _size);
      return _key[id];
    }

    int get_index(const char* word, size_t len) const
    {
      auto result_pair = _da->exactMatchSearch<result_type>(word, len);
      return result_pair.value;
    }

    int get_index(const std::string& word) const
    { return get_index(word.data(), word.size()); }

    bool count(const char* word, size_t len) const
    { return get_index(word, len) < 0 ? false : true; }

    bool count(const std::string& word) const
    { return get_index(word) < 0 ? false : true; }

    void insert(const char* word, size_t len)
    {
      if (get_index(word, len) < 0)
      {
        _da->update(word, len, _size ++);
        _key.emplace_back(std::string(word, len));
      }
    }

    void insert(const std::string& word)
    {
      if (get_index(word) < 0)
      {
        _da->update(word.data(), word.size(), _size ++);
        _key.emplace_back(word);
      }
    }

    void insert(const std::vector<std::string>& words)
    {
      for (size_t i = 0; i < words.size(); i++)
        insert(words[i]);
    }

    std::vector<std::pair<size_t, std::string>>
    parse(const std::string& text, size_t max_prefix_matches = 128) const
    {
      auto data = text.data();
      size_t cur = 0, len = text.size();
      std::vector<std::pair<size_t, std::string>> result;
      std::vector<result_type> result_pairs;
      result_pairs.reserve(max_prefix_matches);
      while (cur < len)
      {
        size_t n = _da->commonPrefixSearch(data + cur, result_pairs.data(),
            max_prefix_matches, len - cur);
        for (size_t i = 0; i < n && i < max_prefix_matches; i++)
          result.emplace_back(cur, _key[result_pairs[i].value]);

        if (isascii(data[cur]))
          cur++;
        else
        {
          cur++;
          while (cur < len && (data[cur] & 0xC0) == 0x80)
            cur++;
        }
      }
      return result;
    }

    std::string
    max_prefix(const std::string& text, size_t max_prefix_matches = 128) const
    {
      std::string result;
      std::vector<result_type> result_pairs;
      result_pairs.reserve(max_prefix_matches);
      size_t n = _da->commonPrefixSearch(text.data(), result_pairs.data(),
          max_prefix_matches, text.size());
      if (n < 1)
        return result;
      return _key[result_pairs[n - 1].value];
    }
  
  private:
    size_t _size;
    std::unique_ptr<dar> _da;
    std::vector<std::string> _key;
};

}
#endif