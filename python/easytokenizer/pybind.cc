/**
 * Copyright (c) 2022-present, Zejun Wang (wangzejunscut@126.com).
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <tokenizer.h>
#include <tuple>

using tokenizer::Offset;
using tokenizer::Token;
namespace py = pybind11;

std::vector<std::string> tokenize(
    tokenizer::Tokenizer& m, const std::string& text) {
  return m.wordpiece_tokenize(text); 
}

std::vector<int32_t> convert_tokens_to_ids(tokenizer::Tokenizer& m,
    const std::vector<std::string>& tokens,
    bool add_cls_sep = false) {
  return m.convert_tokens_to_ids(tokens, add_cls_sep);
}

std::pair<std::vector<int32_t>, std::vector<Offset>> single_encode(
    tokenizer::Tokenizer& m, const std::string& text, 
    bool add_cls_sep = true, bool truncation = true, size_t max_length = 512) {
  std::vector<int32_t> input_ids;
  std::vector<Offset> offsets;
  m.encode(text, input_ids, offsets, add_cls_sep, truncation, max_length);
  return std::pair<std::vector<int32_t>, std::vector<Offset>>(input_ids, offsets);
}

std::tuple<std::vector<std::vector<int32_t>>,
           std::vector<std::vector<int32_t>>,
           std::vector<std::vector<int32_t>>,
           std::vector<std::vector<Offset>>> batch_encode(
    tokenizer::Tokenizer& m, const std::vector<std::string>& texts, int num_threads = 1,
    bool add_cls_sep = true, bool padding = true, bool padding_to_max_length = false,
    bool truncation = true, size_t max_length = 512) {
  std::vector<std::vector<int32_t>> input_ids;
  std::vector<std::vector<int32_t>> token_type_ids;
  std::vector<std::vector<int32_t>> attention_mask;
  std::vector<std::vector<Offset>> offsets;
  m.encode(texts, input_ids, token_type_ids, attention_mask, offsets, num_threads,
    add_cls_sep, padding, padding_to_max_length, truncation, max_length);
  return std::tuple<std::vector<std::vector<int32_t>>,
           std::vector<std::vector<int32_t>>,
           std::vector<std::vector<int32_t>>,
           std::vector<std::vector<Offset>>>(
    input_ids, token_type_ids, attention_mask, offsets);
}

PYBIND11_MODULE(easytokenizer, m) {
  m.doc() = "An efficient and easy-to-use tokenization toolkit.";
  py::class_<tokenizer::BasicTokenizer> BasicTokenizerClass(m, "BasicTokenizer");
  BasicTokenizerClass.def(py::init<bool>(), py::arg("do_lower_case") = true);
  py::class_<tokenizer::Tokenizer>(m, "AutoTokenizer", BasicTokenizerClass)
    .def(py::init<const std::string&, bool>(), "Init AutoTokenizer",
         py::arg("vocab_path"), py::arg("do_lower_case") = true)
    .def("insert", (void (tokenizer::Tokenizer::*)(const std::string&))
        (&tokenizer::Tokenizer::insert), py::arg("token"))
    .def("insert", (void (tokenizer::Tokenizer::*)(const std::vector<std::string>&))
        (&tokenizer::Tokenizer::insert), py::arg("tokens"))
    .def("size", &tokenizer::Tokenizer::size)
    .def("pad_id", &tokenizer::Tokenizer::pad_id)
    .def("cls_id", &tokenizer::Tokenizer::cls_id)
    .def("sep_id", &tokenizer::Tokenizer::sep_id)
    .def("unk_id", &tokenizer::Tokenizer::unk_id)
    .def("mask_id", &tokenizer::Tokenizer::mask_id)
    .def("get_id", &tokenizer::Tokenizer::get_id, py::arg("token"))
    .def("get_token", &tokenizer::Tokenizer::get_token, py::arg("id"))
    .def("count", &tokenizer::Tokenizer::count, py::arg("token"))
    .def("convert_ids_to_tokens", &tokenizer::Tokenizer::convert_ids_to_tokens, py::arg("input_ids")) 
    .def("convert_tokens_to_ids", &convert_tokens_to_ids, 
         py::arg("tokens"), py::arg("add_cls_sep") = false)
    .def("tokenize", &tokenize, py::arg("text"))
    .def("encode", &single_encode, py::arg("text"), py::arg("add_cls_sep") = true,
         py::arg("truncation") = true, py::arg("max_length") = 512)
    .def("encode", &batch_encode, py::arg("texts"), py::arg("num_threads") = 1,
         py::arg("add_cls_sep") = true, py::arg("padding") = true, 
         py::arg("padding_to_max_length") = false, py::arg("truncation") = true, 
         py::arg("max_length") = 512);
}
