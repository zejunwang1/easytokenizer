/**
 * Copyright (c) 2022-present, Zejun Wang (wangzejunscut@126.com)
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef EASY_TOKENIZER_H
#define EASY_TOKENIZER_H

#include <algorithm>
#include <cmath>
#include <numeric>
#include <thread>
#include <tuple>

#include "dtrie.h"

namespace tokenizer
{

using SizeT   = uint32_t;
using WidthT  = uint_fast8_t;
using Trie    = cedar::DTrie;
using Token   = std::tuple<SizeT, SizeT, std::string>;

static const SizeT _npos = SizeT(-1);

class BasicTokenizer
{
  public:
    BasicTokenizer(bool do_lower_case = true);

    std::vector<Token> basic_tokenize(const std::string& text) const;
    void basic_tokenize(const std::string& text, std::vector<Token>& tokens) const;
    void tokenize(const std::string& text, SizeT pos, std::vector<Token>& tokens) const;

  protected:
    const std::string _pad_token = "[PAD]";
    const std::string _cls_token = "[CLS]";
    const std::string _sep_token = "[SEP]";
    const std::string _unk_token = "[UNK]";
    const std::string _mask_token = "[MASK]";

    bool _do_lower_case;
    const SizeT _max_prefix_matches = 64;

    std::unique_ptr<Trie> _special;
    static std::string normalize(const uint8_t* str) ;
    static int isCntrl(int c) ;
};

class Tokenizer : public BasicTokenizer
{
  public:
    Tokenizer(const std::string& vocab_path, 
              bool do_lower_case = true, 
              bool codepoint_level = true);

    void insert(const std::string& token);
    void insert(const std::vector<std::string>& tokens);

    void add_special_tokens(const std::string& token);
    void add_special_tokens(const std::vector<std::string>& tokens);

    std::string pad_token() const;
    std::string cls_token() const;
    std::string sep_token() const;
    std::string unk_token() const;
    std::string mask_token() const;
    std::string get_token(SizeT id) const;

    SizeT size() const;
    SizeT pad_id() const;
    SizeT cls_id() const;
    SizeT sep_id() const;
    SizeT unk_id() const;
    SizeT mask_id() const;
    SizeT get_id(const std::string& token) const;

    bool count(const std::string& token) const;
    std::vector<std::string> convert_ids_to_tokens(const std::vector<SizeT>& input_ids) const;
    std::vector<SizeT> convert_tokens_to_ids(const std::vector<std::string>& tokens,
        bool add_cls_sep = false) const;
    void convert_tokens_to_ids(const std::vector<std::string>& tokens,
        std::vector<SizeT>& input_ids,
        bool add_cls_sep = false) const;

    // wordpiece tokenize
    std::vector<std::string> wordpiece_tokenize(const std::string& text) const;
    void wordpiece_tokenize(const std::string& text,
        std::vector<std::string>& tokens,
        std::vector<SizeT>& offsets) const;

    // encode single sentence
    std::vector<SizeT> encode(const std::string& text,
        bool add_cls_sep = true,
        bool truncation = true,
        SizeT max_length = 512) const;
    void encode(const std::string& text,
        std::vector<SizeT>& input_ids,
        std::vector<SizeT>& token_type_ids,
        std::vector<SizeT>& attention_mask,
        std::vector<SizeT>& offsets,
        bool add_cls_sep = true,
        bool truncation = true,
        SizeT max_length = 512) const;

    // encode batch sentences
    void encode(const std::vector<std::string>& texts,
        std::vector<std::vector<SizeT>>& input_ids,
        std::vector<std::vector<SizeT>>& token_type_ids,
        std::vector<std::vector<SizeT>>& attention_mask,
        std::vector<std::vector<SizeT>>& offsets,
        int num_threads = 1,
        bool add_cls_sep = true,
        bool padding = true,
        bool padding_to_max_length = false,
        bool truncation = true,
        SizeT max_length = 512) const;
  
  protected:
    std::unique_ptr<Trie> _vocab;
    bool _codepoint_level = true;
    SizeT _pad_id, _cls_id, _sep_id, _unk_id, _mask_id;
    static const SizeT _max_input_chars_per_word = 100;

    void load_vocab(const std::string& vocab_path);
    static bool isAlnum(const char* str, SizeT len) ;
    void build_pos_map(const char* str, SizeT len, 
        std::vector<SizeT>& pos_map) const;
    static void build_index_map(const std::string& text,
        std::vector<SizeT>& byte2index) ;

    static SizeT NFD_codepoint_number(const uint8_t* str) ;
    static SizeT get_codepoint_number(const std::string& token) ;
    static SizeT get_codepoint_number(const char* str, SizeT len) ;
    static WidthT get_num_bytes_of_utf8_char(const char* str, SizeT len) ;

    static SizeT search(const char* str, SizeT len, SizeT index) ;
};

}
#endif
