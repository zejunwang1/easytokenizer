/**
 * Copyright (c) 2022-present, Zejun Wang (wangzejunscut@126.com)
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "tokenizer.h"
#include "utf8proc.h"

namespace tokenizer
{

BasicTokenizer::BasicTokenizer(bool do_lower_case) : _do_lower_case(do_lower_case)
{
  _special = std::unique_ptr<Trie>(new Trie());
  _special->insert(_pad_token);
  _special->insert(_cls_token);
  _special->insert(_sep_token);
  _special->insert(_unk_token);
  _special->insert(_mask_token);
}

void BasicTokenizer::basic_tokenize(const std::string& text,
    std::vector<Token>& tokens) const
{
  if (tokens.size())
    tokens.clear();

  tokens.reserve(text.size());
  auto matches = _special->parse(text, _max_prefix_matches);
  if (matches.empty())
  {
    tokenize(text, 0, tokens);
    return;
  }

  SizeT start = 0;
  std::string subtext;
  for (SizeT i = 0; i < matches.size(); i++)
  {
    subtext = text.substr(start, matches[i].first - start);
    if (subtext.size())
    {
      tokenize(subtext, start, tokens);
      start += subtext.size();
    }
    tokens.emplace_back(start, start + matches[i].second.size(), matches[i].second);
    start += matches[i].second.size();
  }
  if (start < text.size())
  {
    subtext = text.substr(start);
    tokenize(subtext, start, tokens);
  }
}

std::vector<Token> 
BasicTokenizer::basic_tokenize(const std::string& text) const
{
  std::vector<Token> tokens;
  basic_tokenize(text, tokens);
  return tokens;
}

void BasicTokenizer::tokenize(const std::string& text, SizeT pos,
    std::vector<Token>& tokens) const
{
  int32_t unicode = 0;
  bool last_state = false;
  auto data = text.c_str();
  SizeT i = 0, m = 0, n = 0, start = 0, len = text.size();

  char* word = new char[len + 1];
  uint8_t* ch = new uint8_t[8];
  while (i < len)
  {
    if (isascii(data[i])) 
    {
      if (isalnum(data[i])) 
      {
        word[n++] = (_do_lower_case) ? std::tolower(data[i++]) : data[i++];
        continue;
      }
      
      if (n == 0) 
      {
        word[n++] = data[i++];
        word[n] = '\0';
        start = i - n;
        if (!isCntrl(word[0])) 
        {
          last_state = false;
          if (!isspace(word[0])) 
            tokens.emplace_back(pos + start, pos + i, std::string(word, n));
        }
        n = 0;
      }

      if (n > 0) 
      {
        word[n] = '\0';
        start = i - n;
        if (!last_state)
          tokens.emplace_back(pos + start, pos + i, std::string(word, n));
        else 
        {
          std::get<1>(tokens.back()) = pos + i;
          std::get<2>(tokens.back()).append(word, n);
        }
        n = 0;
        last_state = true;
      }
    }
    else
    {
      if (n > 0) 
      {
        word[n] = '\0';
        start = i - n;
        if (!last_state)
          tokens.emplace_back(pos + start, pos + i, std::string(word, n));
        else
        {
          std::get<1>(tokens.back()) = pos + i;
          std::get<2>(tokens.back()).append(word, n);
        }
        n = 0;
        last_state = true;
      }

      word[n++] = data[i++];
      while (i < len && (data[i] & 0xC0) == 0x80)
        word[n++] = data[i++];
      word[n] = '\0';
      start = i - n;

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
          (utf8proc_category_string(unicode)[0] == 'P')) 
      {
        tokens.emplace_back(pos + start, pos + i, std::string(word, n));
        last_state = false;
      } else if (strcmp(utf8proc_category_string(unicode), "Zs") == 0)
        last_state = false;
      else if ((unicode == 0xFFFD) || (utf8proc_category_string(unicode)[0] == 'C'))
        n = 0;
      else
      {
        if (_do_lower_case) 
        {
          m = utf8proc_encode_char(utf8proc_tolower(unicode), ch);
          ch[m] = '\0';

          auto token = normalize(ch);
          if (token.size() == 1 && !isalnum(token[0]) && !isCntrl(token[0]))
          {
            last_state = false;
            if (!isspace(token[0]))
              tokens.emplace_back(pos + start, pos + i, token);
            n = 0;
            continue;
          }
          
          if (!last_state)
            tokens.emplace_back(pos + start, pos + i, token);
          else
          {
            std::get<1>(tokens.back()) = pos + i;
            std::get<2>(tokens.back()).append(token);
          }
        }
        else
        {
          if (!last_state)
            tokens.emplace_back(pos + start, pos + i, std::string(word, n));
          else
          {
            std::get<1>(tokens.back()) = pos + i;
            std::get<2>(tokens.back()).append(word, n);
          }
        }
        last_state = true;
      }
      n = 0;
    }
  }
  if (n > 0) 
  {
    word[n] = '\0';
    start = i - n;
    if (!last_state)
      tokens.emplace_back(pos + start, pos + i, word);
    else
    {
      std::get<1>(tokens.back()) = pos + i;
      std::get<2>(tokens.back()).append(word, n);
    }
  }
  delete []ch;
  delete []word;
}

std::string BasicTokenizer::normalize(const uint8_t* str) const
{
  auto norm = utf8proc_NFD(str);
  SizeT len = strlen((char*)norm);
  std::string result;
  result.reserve(len);
  int32_t unicode = 0;
  SizeT i = 0, n = 0;
  while (i < len)
  {
    if (isascii(norm[i]))
    {
      result.push_back(norm[i]);
      i++;
    }
    else
    {
      i++;
      n++;
      while (i < len && (norm[i] & 0xC0) == 0x80)
      {
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

int BasicTokenizer::isCntrl(int c) const
{
  if (c == '\t' || c == '\r' || c == '\n')
    return 0;
  return iscntrl(c);
}

Tokenizer::Tokenizer(const std::string& vocab_path, bool do_lower_case, bool codepoint_level)
: BasicTokenizer(do_lower_case), _codepoint_level(codepoint_level)
{
  _vocab = std::unique_ptr<Trie>(new Trie());
  load_vocab(vocab_path);

  _pad_id = SizeT(_vocab->get_index(_pad_token));
  _cls_id = SizeT(_vocab->get_index(_cls_token));
  _sep_id = SizeT(_vocab->get_index(_sep_token));
  _unk_id = SizeT(_vocab->get_index(_unk_token));
  _mask_id = SizeT(_vocab->get_index(_mask_token));
}

void Tokenizer::load_vocab(const std::string& vocab_path)
{
  std::ifstream ifs(vocab_path);
  if (!ifs.is_open())
    throw std::invalid_argument(vocab_path + " can not be opened for loading!");
  std::string token;
  while (std::getline(ifs, token))
    if (!token.empty())
      _vocab->insert(token);
  ifs.close();
}

bool Tokenizer::isAlnum(const char* str, SizeT len) const
{
  for (SizeT i = 0; i < len; i++)
    if (!isalnum(str[i]))
      return false;
  return true;
}

void Tokenizer::build_pos_map(const char* str, SizeT len, 
    std::vector<SizeT>& pos_map) const
{
  int32_t unicode = 0;
  uint8_t* ch = new uint8_t[8];
  SizeT cur = 0, val = 0, m = 0, n = 0;
  while (cur < len)
  {
    if (isascii(str[cur]))
    {
      if (isCntrl(str[cur]))
      {
        val++;
        cur++;
        continue;
      }
      pos_map.emplace_back(val);
      val++;
      cur++;
    }
    else
    {
      n++;
      cur++;
      while (cur < len && (str[cur] & 0xC0) == 0x80)
      {
        n++;
        cur++;
      }

      utf8proc_iterate((const uint8_t*)(str + cur - n), n, &unicode);
      if ((unicode == 0xFFFD) || (utf8proc_category_string(unicode)[0] == 'C'))
      {
        val++;
        n = 0;
        continue; 
      }

      if (_do_lower_case)
      {
        m = utf8proc_encode_char(utf8proc_tolower(unicode), ch);
        ch[m] = '\0';
        for (SizeT i = 0; i < NFD_codepoint_number(ch); i++)
          pos_map.emplace_back(val);
      }
      else
        pos_map.emplace_back(val);

      val++;
      n = 0;
    }
  }
  pos_map.emplace_back(val);
  delete []ch;
}

SizeT Tokenizer::NFD_codepoint_number(const uint8_t* str) const
{
  auto norm = utf8proc_NFD(str);
  SizeT len = strlen((char*)norm);
  int32_t unicode = 0;
  SizeT i = 0, n = 0, c = 0;
  while (i < len)
  {
    if (isascii(norm[i]))
    {
      i++;
      c++;
    }
    else
    {
      i++;
      n++;
      while (i < len && (norm[i] & 0xC0) == 0x80)
      {
        i++;
        n++;
      }
      utf8proc_iterate(norm + i - n, n, &unicode);
      if (strcmp(utf8proc_category_string(unicode), "Mn") != 0)
        c++;
      n = 0;
    }
  }
  free(norm);
  return c;
}

WidthT Tokenizer::get_num_bytes_of_utf8_char(const char* str, SizeT len) const
{
  SizeT cur = 1;
  WidthT num_bytes = 1;
  while (cur < len && (str[cur++] & 0xC0) == 0x80)
    num_bytes++;
  return num_bytes;
}

SizeT Tokenizer::get_codepoint_number(const char* str, SizeT len) const
{
  SizeT cur_bytes = 0, cur_index = 0;
  while (cur_bytes < len) 
  {
    cur_bytes += get_num_bytes_of_utf8_char(str + cur_bytes, len - cur_bytes);
    cur_index ++;
  }
  return cur_index;
}

SizeT Tokenizer::get_codepoint_number(const std::string& token) const
{ return get_codepoint_number(token.data(), token.size()); }

void Tokenizer::build_index_map(const std::string& text, 
    std::vector<SizeT>& byte2index) const
{
  auto data = text.c_str();
  SizeT cur_bytes = 0, cur_index = 0, len = text.size();
  byte2index.resize(len + 1, _npos);
  while (cur_bytes < len)
  {
    byte2index[cur_bytes] = cur_index++;
    cur_bytes += get_num_bytes_of_utf8_char(data + cur_bytes, len - cur_bytes);
  }
  byte2index[cur_bytes] = cur_index;
}

SizeT Tokenizer::search(const char* str, SizeT len, SizeT index) const
{
  SizeT cur_bytes = 0, cur_index = 0;
  while (cur_bytes < len)
  {
    if (cur_index == index)
      return cur_bytes;
    cur_bytes += get_num_bytes_of_utf8_char(str + cur_bytes, len - cur_bytes);
    cur_index ++;
  }
  if (cur_index == index)
    return len;
  return _npos;
}

void Tokenizer::insert(const std::string& token)
{ _vocab->insert(token); }

void Tokenizer::insert(const std::vector<std::string>& tokens)
{ _vocab->insert(tokens); }

void Tokenizer::add_special_tokens(const std::string& token)
{ _special->insert(token); }

void Tokenizer::add_special_tokens(const std::vector<std::string>& tokens)
{ _special->insert(tokens); }

std::string Tokenizer::pad_token() const
{ return _pad_token; }

std::string Tokenizer::cls_token() const
{ return _cls_token; }

std::string Tokenizer::sep_token() const
{ return _sep_token; }

std::string Tokenizer::unk_token() const
{ return _unk_token; }

std::string Tokenizer::mask_token() const
{ return _mask_token; }

std::string Tokenizer::get_token(SizeT id) const
{ return _vocab->get_key(id); }

SizeT Tokenizer::size() const
{ return _vocab->size(); }

SizeT Tokenizer::pad_id() const
{ return _pad_id; }

SizeT Tokenizer::cls_id() const
{ return _cls_id; }

SizeT Tokenizer::sep_id() const
{ return _sep_id; }

SizeT Tokenizer::unk_id() const
{ return _unk_id; }

SizeT Tokenizer::mask_id() const
{ return _mask_id; }

SizeT Tokenizer::get_id(const std::string& token) const
{
  int id = _vocab->get_index(token);
  return id < 0 ? _unk_id : id;
}

bool Tokenizer::count(const std::string& token) const
{ return _vocab->count(token); }

std::vector<std::string>
Tokenizer::convert_ids_to_tokens(const std::vector<SizeT>& input_ids) const
{
  std::vector<std::string> tokens;
  tokens.reserve(input_ids.size());
  for (SizeT i = 0; i < input_ids.size(); i++)
    tokens.emplace_back(get_token(input_ids[i]));
  return tokens;
}

std::vector<SizeT> Tokenizer::convert_tokens_to_ids(
    const std::vector<std::string>& tokens, bool add_cls_sep) const
{
  std::vector<SizeT> input_ids;
  input_ids.reserve(tokens.size() + 2);
  convert_tokens_to_ids(tokens, input_ids, add_cls_sep);
  return input_ids;
}

void Tokenizer::convert_tokens_to_ids(const std::vector<std::string>& tokens,
    std::vector<SizeT>& input_ids,
    bool add_cls_sep) const
{
  if (add_cls_sep)
    input_ids.emplace_back(_cls_id);
  for (SizeT i = 0; i < tokens.size(); i++)
    input_ids.emplace_back(get_id(tokens[i]));
  if (add_cls_sep)
    input_ids.emplace_back(_sep_id);
}

void Tokenizer::wordpiece_tokenize(const std::string& text,
    std::vector<std::string>& tokens,
    std::vector<SizeT>& offsets) const
{
  if (tokens.size())
    tokens.clear();
  if (offsets.size())
    offsets.clear();
  tokens.reserve(text.size());
  offsets.reserve(2 * text.size());

  std::vector<Token> base_tokens;
  basic_tokenize(text, base_tokens);

  std::vector<SizeT> byte2index;
  if (_codepoint_level)
    build_index_map(text, byte2index);

  bool is_bad = false;
  auto data = text.c_str();
  SizeT start = 0, end = 0, cur = 0, pos = 0, len = 0, num = 0;
  std::string token, prefix, subtoken;
  std::vector<SizeT> pos_map;
  std::vector<Token> sub_tokens;
  pos_map.reserve(_max_input_chars_per_word);
  sub_tokens.reserve(_max_input_chars_per_word);
  for (SizeT i = 0; i < base_tokens.size(); i++) 
  {
    start = std::get<0>(base_tokens[i]);
    end   = std::get<1>(base_tokens[i]);
    token = std::get<2>(base_tokens[i]);
    
    if (_special->count(token) || _vocab->count(token)) 
    {
      tokens.emplace_back(token);
      if (_codepoint_level)
      {
        offsets.emplace_back(byte2index[start]);
        offsets.emplace_back(byte2index[end]);
      }
      else
      {
        offsets.emplace_back(start);
        offsets.emplace_back(end);
      }
      continue;
    }

    if (token.size() > _max_input_chars_per_word)
    {
      tokens.emplace_back(_unk_token);
      if (_codepoint_level)
      {
        offsets.emplace_back(byte2index[start]);
        offsets.emplace_back(byte2index[end]);
      }
      else
      {
        offsets.emplace_back(start);
        offsets.emplace_back(end);
      }
      continue;
    }

    // wordpiece tokenize
    cur = 0;
    pos = 0;
    is_bad = false;
    sub_tokens.clear();
    while (cur < token.size()) 
    {
      subtoken = token.substr(cur);
      if (cur > 0)  
        subtoken = "##" + subtoken;

      prefix = _vocab->max_prefix(subtoken, _max_prefix_matches);
      if ((cur > 0 && prefix.size() < 3) || prefix.empty()) 
      {
        is_bad = true;
        break;
      }

      len = cur > 0 ? (prefix.size() - 2) : prefix.size();
      num = cur > 0 ? get_codepoint_number(prefix) - 2 :
                      get_codepoint_number(prefix);
      
      sub_tokens.emplace_back(pos, pos + num, prefix);
      cur += len;
      pos += num;
    }

    if (is_bad) 
    {
      tokens.emplace_back(_unk_token);
      if (_codepoint_level)
      {
        offsets.emplace_back(byte2index[start]);
        offsets.emplace_back(byte2index[end]);
      }
      else
      {
        offsets.emplace_back(start);
        offsets.emplace_back(end);
      }
      continue;
    }

    if (isAlnum(data + start, end - start))
    {
      for (SizeT j = 0; j < sub_tokens.size(); j++)
      {
        tokens.emplace_back(std::get<2>(sub_tokens[j]));
        if (_codepoint_level)
        {
          offsets.emplace_back(byte2index[start] + std::get<0>(sub_tokens[j]));
          offsets.emplace_back(byte2index[start] + std::get<1>(sub_tokens[j]));
        }
        else
        {
          offsets.emplace_back(start + std::get<0>(sub_tokens[j]));
          offsets.emplace_back(start + std::get<1>(sub_tokens[j]));
        }
      }
      continue;
    }

    pos_map.clear();
    build_pos_map(data + start, end - start, pos_map);
    for (SizeT j = 0; j < sub_tokens.size(); j++)
    {
      auto a = std::get<0>(sub_tokens[j]);
      auto b = std::get<1>(sub_tokens[j]);
      b = (pos_map[a] == pos_map[b]) ? (pos_map[a] + 1) : pos_map[b];
      a = pos_map[a];

      tokens.emplace_back(std::get<2>(sub_tokens[j]));
      if (_codepoint_level)
      {
        offsets.emplace_back(byte2index[start] + a);
        offsets.emplace_back(byte2index[start] + b);
      }
      else
      {
        offsets.emplace_back(start + search(data + start, end - start, a));
        offsets.emplace_back(start + search(data + start, end - start, b));
      }
    }
  }
}

std::vector<std::string> 
Tokenizer::wordpiece_tokenize(const std::string& text) const
{
  std::vector<std::string> tokens;
  std::vector<SizeT> offsets;
  wordpiece_tokenize(text, tokens, offsets);
  return tokens;
}

void Tokenizer::encode(const std::string& text,
    std::vector<SizeT>& input_ids,
    std::vector<SizeT>& token_type_ids,
    std::vector<SizeT>& attention_mask,
    std::vector<SizeT>& offsets,
    bool add_cls_sep,
    bool truncation,
    SizeT max_length) const
{
  if (input_ids.size())
    input_ids.clear();
  if (token_type_ids.size())
    token_type_ids.clear();
  if (attention_mask.size())
    attention_mask.clear();
  if (offsets.size())
    offsets.clear();

  std::vector<std::string> tokens;
  wordpiece_tokenize(text, tokens, offsets);

  // input_ids
  SizeT n = tokens.size();
  SizeT capacity = std::max(max_length, n + 2);
  input_ids.reserve(capacity);
  convert_tokens_to_ids(tokens, input_ids, add_cls_sep);
  
  // truncation
  if (truncation && input_ids.size() > max_length)
  {
    n = max_length + offsets.size() - input_ids.size();
    input_ids.resize(max_length);
    offsets.resize(n);
    if (add_cls_sep)
      input_ids[max_length - 1] = _sep_id;
    if (input_ids.capacity() > max_length * 32)
      std::vector<SizeT>(input_ids).swap(input_ids);
  }  
  
  // token_type_ids and attention_mask
  n = input_ids.size();
  capacity = std::max(max_length, n);
  token_type_ids.reserve(capacity);
  attention_mask.reserve(capacity);
  token_type_ids.resize(n, 0);
  attention_mask.resize(n, 1);
}

std::vector<SizeT> Tokenizer::encode(const std::string& text, 
    bool add_cls_sep, 
    bool truncation, 
    SizeT max_length) const
{
  const auto& tokens = wordpiece_tokenize(text);
  SizeT capacity = std::max(max_length, SizeT(tokens.size() + 2));
  std::vector<SizeT> input_ids;
  input_ids.reserve(capacity);
  convert_tokens_to_ids(tokens, input_ids, add_cls_sep);

  // truncation
  if (truncation && input_ids.size() > max_length)
  {
    input_ids.resize(max_length);
    if (add_cls_sep)
      input_ids[max_length - 1] = _sep_id;
    if (input_ids.capacity() > max_length * 32)
      std::vector<SizeT>(input_ids).swap(input_ids);
  }
  return input_ids;
}

void Tokenizer::encode(const std::vector<std::string>& texts,
    std::vector<std::vector<SizeT>>& input_ids,
    std::vector<std::vector<SizeT>>& token_type_ids,
    std::vector<std::vector<SizeT>>& attention_mask,
    std::vector<std::vector<SizeT>>& offsets,
    int num_threads,
    bool add_cls_sep,
    bool padding,
    bool padding_to_max_length,
    bool truncation,
    SizeT max_length) const
{
  if (input_ids.size())
    input_ids.clear();
  if (token_type_ids.size())
    token_type_ids.clear();
  if (attention_mask.size())
    attention_mask.clear();
  if (offsets.size())
    offsets.clear();

  // input_ids
  SizeT n = texts.size();
  input_ids.resize(n);
  token_type_ids.resize(n);
  attention_mask.resize(n);
  offsets.resize(n);
  
  if (num_threads <= 1)
    for (SizeT i = 0; i < n; i++)
      encode(texts[i], input_ids[i], token_type_ids[i], attention_mask[i], offsets[i],
        add_cls_sep, truncation, max_length);
  else
  {
    // Multithreading Implementation
    #ifdef WITH_OMP
    #pragma omp parallel for num_threads(num_threads)
    for (SizeT i = 0; i < n; i++)
      encode(texts[i], input_ids[i], token_type_ids[i], attention_mask[i], offsets[i], 
        add_cls_sep, truncation, max_length);
    #else
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(num_threads));
    auto func = [&](SizeT start_index, SizeT end_index)
    {
      for (SizeT i = start_index; i < end_index; i++)
        encode(texts[i], input_ids[i], token_type_ids[i], attention_mask[i], offsets[i], 
          add_cls_sep, truncation, max_length);
    };
    SizeT start = 0, end = 0, step = ceil(n / float(num_threads));
    for (int i = 0; i < num_threads; i++)
    {
      end = start + step;
      if (end > n)
        end = n;
      threads.emplace_back(std::thread(func, start, end));
      start = end;
    }

    for (auto& t : threads)
      t.join();
    #endif
  }

  if (!padding)
    return;
  
  // padding
  SizeT max_seq_len = std::accumulate(input_ids.begin(), input_ids.end(), 0,
    [](size_t len, const std::vector<SizeT>& input)
    { return std::max(len, input.size()); });
  SizeT seq_len = padding_to_max_length ? max_length : max_seq_len;
  for (SizeT i = 0; i < n; i++)
  {
    input_ids[i].resize(seq_len, _pad_id);
    token_type_ids[i].resize(seq_len);
    attention_mask[i].resize(seq_len);
  }
}

}
