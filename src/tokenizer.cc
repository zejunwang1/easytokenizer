/**
 * Copyright (c) 2022-present, Zejun Wang (wangzejunscut@126.com).
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "tokenizer.h"
#include "utf8proc.h"

namespace tokenizer {

BasicTokenizer::BasicTokenizer(bool do_lower_case) : _do_lower_case(do_lower_case) {
  _special = std::unique_ptr<trie>(new trie());
  _special->insert(_pad_token);
  _special->insert(_cls_token);
  _special->insert(_sep_token);
  _special->insert(_unk_token);
  _special->insert(_mask_token);
}

std::string BasicTokenizer::pad_token() const {
  return _pad_token;
}

std::string BasicTokenizer::cls_token() const {
  return _cls_token;
}

std::string BasicTokenizer::sep_token() const {
  return _sep_token;
}

std::string BasicTokenizer::unk_token() const {
  return _unk_token;
}

std::string BasicTokenizer::mask_token() const {
  return _mask_token;
}

void BasicTokenizer::add_special_tokens(const std::string& token) {
  _special->insert(token);
}

void BasicTokenizer::add_special_tokens(const std::vector<std::string>& tokens) {
  for (size_t i = 0; i < tokens.size(); i++) {
    _special->insert(tokens[i]);
  }
}

std::vector<std::string> BasicTokenizer::basic_tokenize(const std::string& text) {
  std::vector<std::string> res;
  std::vector<Token> tokens;
  basic_tokenize(text, tokens);
  res.reserve(tokens.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    res.emplace_back(tokens[i].second);
  }
  return res;
}

void BasicTokenizer::basic_tokenize(const std::string& text, std::vector<Token>& tokens) {
  if (tokens.size())  tokens.clear();
  tokens.reserve(text.size());
  auto matches = _special->parse(text, _max_prefix_matches);
  if (matches.empty()) {
    tokenize(text, 0, tokens);
    return;
  }
  size_t start = 0;
  std::string subtext;
  for (size_t i = 0; i < matches.size(); i++) {
    subtext = text.substr(start, matches[i].first - start);
    if (subtext.size()) {
      tokenize(subtext, start, tokens);
      start += subtext.size();
    }
    tokens.emplace_back(start, matches[i].second);
    start += matches[i].second.size();
  }
  if (start < text.size()) {
    subtext = text.substr(start);
    tokenize(subtext, start, tokens);
  }
}

void BasicTokenizer::tokenize(const std::string& text, size_t pos, std::vector<Token>& tokens) {
  const char* buf = text.c_str();
  size_t len = text.size();
  char* word = new char[len + 1];
  uint8_t* ch = new uint8_t[10];
  
  int32_t unicode = 0;
  bool last_state = false;
  size_t i = 0, m = 0, n = 0, start = 0, last_index = 0;
  while (i < len) {
    if (isascii(buf[i])) {
      if (std::isalnum(buf[i])) {
        word[n++] = (_do_lower_case) ? std::tolower(buf[i++]) : buf[i++];
        continue;
      }
      
      if (n == 0) {
        word[n++] = buf[i++];
        word[n] = '\0';
        start = i - n;
        if (!(isCntrl(word[0]) || word[0] == 0)) {
          last_state = false;
          if (!std::isspace(word[0])) {
            tokens.emplace_back(pos + start, std::string(word, n));
          }
        }
        n = 0;
      }
      
      if (n > 0) {
        word[n] = '\0';
        start = i - n;
        if (!last_state) {
          tokens.emplace_back(pos + start, std::string(word, n));
        } else {
          last_index = tokens.size() - 1;
          tokens[last_index].second.append(word, n);
        }
        n = 0;
        last_state = true;
      }
    } else {
      if (n > 0) {
        word[n] = '\0';
        start = i - n;
        if (!last_state) {
          tokens.emplace_back(pos + start, std::string(word, n));
        } else {
          last_index = tokens.size() - 1;
          tokens[last_index].second.append(word, n);
        }
        n = 0;
        last_state = true;
      }
      
      word[n++] = buf[i++];
      while (i < len && (buf[i] & 0xC0) == 0x80) {
        word[n++] = buf[i++];
      }
      word[n] = '\0';
      
      // unicode value
      utf8proc_iterate((uint8_t*)word, n, &unicode);
      // is chinese character
      if ((unicode >= 0x4E00 && unicode <= 0x9FFF) ||
          (unicode >= 0x3400 && unicode <= 0x4DBF) ||
          (unicode >= 0x20000 && unicode <= 0x2A6DF) ||
          (unicode >= 0x2A700 && unicode <= 0x2B73F) ||
          (unicode >= 0x2B740 && unicode <= 0x2B81F) ||
          (unicode >= 0x2B820 && unicode <= 0x2CEAF) ||
          (unicode >= 0xF900 && unicode <= 0xFAFF) ||
          (unicode >= 0x2F800 && unicode <= 0x2FA1F) ||
          (utf8proc_category_string(unicode)[0] == 'P')) {
        tokens.emplace_back(pos + i - n, std::string(word, n));
        last_state = false;
      } else if (strcmp(utf8proc_category_string(unicode), "Zs") == 0) {
        last_state = false;
      } else if ((unicode == 0xFFFD) || (utf8proc_category_string(unicode)[0] == 'C')) {
        n = 0;
      } else {
        if (_do_lower_case) {
          m = utf8proc_encode_char(utf8proc_tolower(unicode), ch);
          ch[m] = '\0';
          if (!last_state) {
            tokens.emplace_back(pos + i - n, normalize(ch));
          } else {
            last_index = tokens.size() - 1;
            tokens[last_index].second.append(normalize(ch));
          }
        } else {
          if (!last_state) {
            tokens.emplace_back(pos + i - n, std::string(word, n));
          } else {
            last_index = tokens.size() - 1;
            tokens[last_index].second.append(word, n);
          }
        }
        last_state = true;
      }
      n = 0;
    }
  }
  if (n > 0) {
    word[n] = '\0';
    start = i - n;
    if (!last_state) {
      tokens.emplace_back(pos + start, word);
    } else {
      last_index = tokens.size() - 1;
      tokens[last_index].second.append(word, n);
    }
  }
  delete []ch;
  delete []word;
}

int BasicTokenizer::isCntrl(int c) {
  if (c == '\t' || c == '\n' || c == '\r')  return 0;
  return std::iscntrl(c);
}

std::string BasicTokenizer::normalize(const uint8_t* str) {
  auto norm = utf8proc_NFD(str);
  size_t len = strlen((char*)norm);
  std::string result;
  result.reserve(len);
  int32_t unicode = 0;
  size_t i = 0, n = 0;
  while (i < len) {
    if (isascii(norm[i])) {
      result.push_back(norm[i]);
      i++;
    } else {
      i++;
      n++;
      while (i < len && (norm[i] & 0xC0) == 0x80) {
        i++;
        n++;
      }
      utf8proc_iterate(norm + i - n, n, &unicode);
      if (strcmp(utf8proc_category_string(unicode), "Mn") != 0)
        result.append((char*)(norm + i - n), n);
      n = 0;
    }
  }
  free(norm);
  return result;
}

Tokenizer::Tokenizer(const std::string& vocab_path, bool do_lower_case)
: BasicTokenizer(do_lower_case) {
  _vocab = std::unique_ptr<trie>(new trie());
  load_vocab(vocab_path);
  
  _pad_id = _vocab->index(_pad_token);
  _cls_id = _vocab->index(_cls_token);
  _sep_id = _vocab->index(_sep_token);
  _unk_id = _vocab->index(_unk_token);
  _mask_id = _vocab->index(_mask_token);
}

void Tokenizer::load_vocab(const std::string& vocab_path) {
  std::ifstream ifs(vocab_path);
  if (!ifs.is_open()) {
    throw std::invalid_argument(vocab_path + " can not be opened for loading!\n");
  }
  std::string token;
  while (std::getline(ifs, token)) {
    if (token.empty()) continue;
    _vocab->insert(token);
  }
  ifs.close();
}

void Tokenizer::insert(const std::string& token) {
  _vocab->insert(token);
}

void Tokenizer::insert(const std::vector<std::string>& tokens) {
  for (size_t i = 0; i < tokens.size(); i++) {
    _vocab->insert(tokens[i]);
  }
}

size_t Tokenizer::size() const {
  return _vocab->size();
}

int32_t Tokenizer::pad_id() const {
  return _pad_id;
}

int32_t Tokenizer::cls_id() const {
  return _cls_id;
}

int32_t Tokenizer::sep_id() const {
  return _sep_id;
}

int32_t Tokenizer::unk_id() const {
  return _unk_id;
}

int32_t Tokenizer::mask_id() const {
  return _mask_id;
}

int32_t Tokenizer::get_id(const std::string& token) {
  int id = _vocab->index(token);
  return id < 0 ? _unk_id : id;
}

std::string Tokenizer::get_token(int32_t id) {
  return _vocab->keys(id);
}

bool Tokenizer::count(const std::string& token) const {
  return _vocab->count(token);
}

std::vector<std::string> Tokenizer::convert_ids_to_tokens(const std::vector<int32_t>& input_ids) {
  std::vector<std::string> tokens;
  tokens.reserve(input_ids.size());
  for (size_t i = 0; i < input_ids.size(); i++) {
    tokens.emplace_back(get_token(input_ids[i]));
  }
  return tokens;
}

std::vector<int32_t> Tokenizer::convert_tokens_to_ids(
    const std::vector<std::string>& tokens, bool add_cls_sep) {
  std::vector<int32_t> input_ids;
  input_ids.reserve(tokens.size() + 2);
  convert_tokens_to_ids(tokens, input_ids, add_cls_sep);
  return input_ids;
}

void Tokenizer::convert_tokens_to_ids(const std::vector<std::string>& tokens,
    std::vector<int32_t>& input_ids,
    bool add_cls_sep)
{
  if (add_cls_sep)
    input_ids.emplace_back(_cls_id);
  for (size_t i = 0; i < tokens.size(); i++)
    input_ids.emplace_back(get_id(tokens[i]));
  if (add_cls_sep)
    input_ids.emplace_back(_sep_id);
}

std::vector<std::string> Tokenizer::wordpiece_tokenize(const std::string& text) {
  std::vector<std::string> tokens;
  std::vector<Offset> offsets;
  wordpiece_tokenize(text, tokens, offsets);
  return tokens;
}

void Tokenizer::wordpiece_tokenize(const std::string& text, 
    std::vector<std::string>& tokens,
    std::vector<Offset>& offsets) {
  if (tokens.size())  tokens.clear();
  if (offsets.size()) offsets.clear();
  tokens.reserve(text.size());
  offsets.reserve(text.size());
  
  std::vector<Token> base_tokens;
  basic_tokenize(text, base_tokens);
  
  bool is_bad = false;
  size_t pos = 0, cur = 0, len = 0;
  std::string token, prefix, subtoken;
  std::vector<std::string> sub_tokens;
  std::vector<Offset> sub_offsets;
  for (size_t i = 0; i < base_tokens.size(); i++) {
    pos = base_tokens[i].first;
    token = base_tokens[i].second;
    if (_special->count(token) || _vocab->count(token) ||
        token.size() > _max_input_chars_per_word) {
      tokens.emplace_back(token);
      offsets.emplace_back(pos, token.size());
      continue;
    }
    
    // wordpiece tokenize
    cur = 0;
    is_bad = false;
    sub_tokens.clear();
    sub_offsets.clear();
    while (cur < token.size()) {
      subtoken = token.substr(cur);
      if (cur > 0)  subtoken = "##" + subtoken;
      prefix = _vocab->max_prefix(subtoken, _max_prefix_matches);
      if ((cur > 0 && prefix.size() < 3) || prefix.empty()) {
        is_bad = true;
        break;
      }

      sub_tokens.emplace_back(prefix);
      len = cur > 0 ? (prefix.size() - 2) : prefix.size();
      sub_offsets.emplace_back(pos + cur, len);
      cur += len;
    }
    if (is_bad) {
      tokens.emplace_back(token);
      offsets.emplace_back(pos, token.size());
      continue;
    }
    for (size_t j = 0; j < sub_tokens.size(); j++) {
      tokens.emplace_back(sub_tokens[j]);
      offsets.emplace_back(sub_offsets[j].first, sub_offsets[j].second);
    }
  }
}

std::vector<int32_t> Tokenizer::encode(const std::string& text, 
    bool add_cls_sep, 
    bool truncation, 
    size_t max_length) {
  std::vector<int32_t> input_ids;
  std::vector<Offset> offsets;
  encode(text, input_ids, offsets, add_cls_sep, truncation, max_length);
  return input_ids;
}

void Tokenizer::encode(const std::string& text,
    std::vector<int32_t>& input_ids, 
    std::vector<Offset>& offsets, 
    bool add_cls_sep, 
    bool truncation, 
    size_t max_length) {
  if (input_ids.size())
    input_ids.clear();
  if (offsets.size())
    offsets.clear();

  std::vector<std::string> tokens;
  wordpiece_tokenize(text, tokens, offsets);
  
  size_t n = tokens.size();
  input_ids.reserve(std::max(max_length, n + 2));
  convert_tokens_to_ids(tokens, input_ids, add_cls_sep);
  
  // truncation
  if (!truncation || input_ids.size() <= max_length)  return;
  n = max_length + offsets.size() - input_ids.size();
  input_ids.resize(max_length);
  offsets.resize(n);
  if (add_cls_sep)
    input_ids[max_length - 1] = _sep_id;
  if (input_ids.capacity() > max_length * 32)
    std::vector<int32_t>(input_ids).swap(input_ids);
}

void Tokenizer::encode(const std::vector<std::string>& texts,
    std::vector<std::vector<int32_t>>& input_ids,
    std::vector<std::vector<int32_t>>& token_type_ids,
    std::vector<std::vector<int32_t>>& attention_mask,
    std::vector<std::vector<Offset>>& offsets,
    int num_threads,
    bool add_cls_sep,
    bool padding,
    bool padding_to_max_length,
    bool truncation,
    size_t max_length) {
  if (input_ids.size()) input_ids.clear();
  if (token_type_ids.size())  token_type_ids.clear();
  if (attention_mask.size())  attention_mask.clear();
  if (offsets.size()) offsets.clear();
  
  // input_ids
  size_t n = texts.size();
  input_ids.resize(n);
  offsets.resize(n);
  if (num_threads <= 1) {
    for (size_t i = 0; i < n; i++) {
      encode(texts[i], input_ids[i], offsets[i], add_cls_sep, truncation, max_length);
    }
  } else {
    // Multithreading Implementation
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(num_threads));
    int avg_num = n / num_threads;
    for (int i = 0; i < num_threads - 1; i++) {
      threads.emplace_back([&, i]() {
        for (int j = i * avg_num; j < (i + 1) * avg_num; j++) {
          encode(texts[j], input_ids[j], offsets[j], add_cls_sep, truncation, max_length);
        }
      });
    }
    threads.emplace_back([&]() {
      for (int j = (num_threads - 1) * avg_num; j < n; j++) {
        encode(texts[j], input_ids[j], offsets[j], add_cls_sep, truncation, max_length);
      }
    });

    for (auto& t : threads) {
      t.join();
    }
  }

  // token_type_ids and attention_mask
  token_type_ids.resize(n);
  attention_mask.resize(n);
  size_t seq_len, capacity;
  for (size_t i = 0; i < n; i++) {
    seq_len = input_ids[i].size();
    capacity = std::max(max_length, seq_len);
    token_type_ids[i].reserve(capacity);
    attention_mask[i].reserve(capacity);
    token_type_ids[i].resize(seq_len, 0);
    attention_mask[i].resize(seq_len, 1);
  }
  if (!padding) return;
  
  // padding
  size_t max_seq_len = std::accumulate(input_ids.begin(), input_ids.end(), 0,
    [](size_t len, const std::vector<int32_t>& input) {
      return std::max(len, input.size());
    });
  seq_len = padding_to_max_length ? max_length : max_seq_len;
  for (size_t i = 0; i < n; i++) {
    input_ids[i].resize(seq_len, _pad_id);
    token_type_ids[i].resize(seq_len);
    attention_mask[i].resize(seq_len);
  }
}

}
