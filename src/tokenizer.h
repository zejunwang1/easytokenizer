/**
 * Copyright (c) 2022-present, Zejun Wang (wangzejunscut@126.com).
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef EASY_TOKENIZER_H
#define EASY_TOKENIZER_H

#include <cctype>
#include <cstring>
#include <memory>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include "dtrie.h"

using trie = cedar::DTrie;

namespace tokenizer {

typedef std::pair<size_t, size_t> Offset;
typedef std::pair<size_t, std::string> Token;

class BasicTokenizer {
  public:
    BasicTokenizer(bool do_lower_case = true);
    std::string pad_token() const;
    std::string cls_token() const;
    std::string sep_token() const;
    std::string unk_token() const;
    std::string mask_token() const;
    
    void add_special_tokens(const std::string& token);
    void add_special_tokens(const std::vector<std::string>& tokens);
    
    std::vector<std::string> basic_tokenize(const std::string& text);
    void basic_tokenize(const std::string& text, std::vector<Token>& tokens);
    void tokenize(const std::string& text, size_t pos, std::vector<Token>& tokens);
  
  protected:
    const std::string _pad_token = "[PAD]";
    const std::string _cls_token = "[CLS]";
    const std::string _sep_token = "[SEP]";
    const std::string _unk_token = "[UNK]";
    const std::string _mask_token = "[MASK]";
    
    bool _do_lower_case;
    const size_t _max_prefix_matches = 512;

    std::unique_ptr<trie> _special;
    std::vector<std::string> split_by_special_tokens(const std::string& text) const;
    std::string normalize(const uint8_t* str);
    int isCntrl(int c);
};


class Tokenizer : public BasicTokenizer {
  public:
    Tokenizer(const std::string& vocab_path, bool do_lower_case = true);
    
    void insert(const std::string& token);
    void insert(const std::vector<std::string>& tokens);
    
    size_t size() const;
    int64_t pad_id() const;
    int64_t cls_id() const;
    int64_t sep_id() const;
    int64_t unk_id() const;
    int64_t mask_id() const;
    int64_t get_id(const std::string& token);
    std::string get_token(int64_t id);

    bool count(const std::string& token) const;    
    bool startswith(const std::string& text, const std::string& str,
        size_t beg = 0, int len = -1);
    
    std::vector<std::string> convert_ids_to_tokens(const std::vector<int64_t>& input_ids);    
    std::vector<int64_t> convert_tokens_to_ids(const std::vector<std::string>& tokens,
        bool add_cls_sep = true);
    
    std::vector<std::string> wordpiece_tokenize(const std::string& text);
    void wordpiece_tokenize(const std::string& text, 
        std::vector<std::string>& tokens, 
        std::vector<Offset>& offsets);
    
    // single text
    std::vector<int64_t> encode(const std::string& text, 
        bool add_cls_sep = true, 
        bool truncation = true, 
        size_t max_length = 512);
    void encode(const std::string& text,
        std::vector<int64_t>& input_ids, 
        std::vector<Offset>& offsets,
        bool add_cls_sep = true, 
        bool truncation = true, 
        size_t max_length = 512);
    
    // batch texts
    void encode(const std::vector<std::string>& texts,
        std::vector<std::vector<int64_t>>& input_ids,
        std::vector<std::vector<int64_t>>& token_type_ids,
        std::vector<std::vector<int64_t>>& attention_mask,
        std::vector<std::vector<Offset>>& offsets,
        int num_threads = 1,
        bool add_cls_sep = true, 
        bool padding = true, 
        bool padding_to_max_length = false,
        bool truncation = true, 
        size_t max_length = 512);
    
  protected:
    std::unique_ptr<trie> _vocab;
    const size_t _max_input_chars_per_word = 100;
    void load_vocab(const std::string& vocab_path);
};

}
#endif  //  EASY_TOKENIZER_H
