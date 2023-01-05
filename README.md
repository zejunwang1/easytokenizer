# easytokenizer-v0.2.0: 高性能文本 Tokenizer 库

easytokenizer 是一个简单易用的高性能文本 Tokenizer 库，支持类似 HuggingFace transformers 中 BertTokenizer 的词语切分和标记化功能。具有如下特点：

- 实现高效，基于双数组字典树 (double-array trie) 和 Unicode 规范化工具 utf8proc

- 支持多线程，在处理大批量文本输入时有一定的加速效果

- 支持 c++ 和 python

## C++

### Demo

使用示例参考 example/cpp/demo.cc，通过 cmake 进行编译：

```shell
git clone https://github.com/zejunwang1/easytokenizer
cd easytokenizer/
mkdir build
cd build/
# 默认使用 c++11 thread 线程库
cmake ..
# 使用 OMP 多线程
# cmake -DWITH_OMP=ON ..     
make -j4
```

执行上述命令后，会在 build/examples/cpp 文件夹下生成可执行文件 demo

```shell
./examples/cpp/demo -h
```

显示帮助信息：

```
./examples/cpp/demo {OPTIONS}

    easytokenizer-cpp usage demo.

  OPTIONS:

      -h, --help                        Show help information
      --vocab_path                      Tokenizer vocabulary file.
      --do_lower_case                   Whether to convert upper case letters to
                                        lower case.
      --codepoint_level                 Whether to return character position in
                                        offsets.
```

```shell
./examples/cpp/demo --vocab_path ../data/bert-base-chinese-vocab.txt --do_lower_case
```

运行后部分结果如下：

```
encode batch texts:
计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。
清华大学的[MASK]算机科学与技术专业实力全国第一。
encode result:
input_ids:
101 6369 5050 3322 4906 2110 680 2825 3318 8020 8134 11300 8196 9982 8256 11061 8021 3221 671 7305 3249 6858 7770 5023 2110 3413 3315 4906 683 689 511 102 
101 3926 1290 1920 2110 4638 103 5050 3322 4906 2110 680 2825 3318 683 689 2141 1213 1059 1744 5018 671 511 102 0 0 0 0 0 0 0 0 
attention_mask:
1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 
offsets:
0 3 3 6 6 9 9 12 12 15 15 18 18 21 21 24 24 27 27 30 30 33 33 35 36 43 44 47 48 58 58 61 61 64 64 67 67 70 70 73 73 76 76 79 79 82 82 85 85 88 88 91 91 94 94 97 97 100 100 103 
0 3 3 6 6 9 9 12 12 15 15 21 21 24 24 27 27 30 30 33 33 36 36 39 39 42 42 45 45 48 48 51 51 54 54 57 57 60 60 63 63 66 66 69
```

offsets 表示 input_ids 中除 [CLS] 和 [SEP] 外的其他有效 token 在原字符串中的字符/字节位置。

- 当设置 add_cls_sep=true，codepoint_level=false 时，input_ids 中第 i 个 token 在原字符串中的起始字节位置为 offsets[2 \* (i - 1)]，终止字节位置为 offsets[2 \* (i - 1) + 1]；

- 当设置 add_cls_sep=false，codepoint_level=false 时，input_ids 中第 i 个 token 在原字符串中的起始字节位置为 offsets[2 \* i]，终止字节位置为 offsets[2 \* i + 1]。

### Speed

在 data 文件夹中包含了速度测试需要用到的句子文件 sents.txt 和 sents_17w.txt。sents.txt 为从中文维基百科中抽取的 10098 条句子（平均长度在 128 个字符以上），sents_17w.txt 为从中文维基百科中抽取的 179608 条句子。使用 build/examples/cpp 文件夹下生成的 speed_tests 测试 c++ 下的处理速度：

```
./examples/cpp/speed_tests {OPTIONS}

    easytokenizer-cpp speed testing.

  OPTIONS:

      -h, --help                        Show help information
      --vocab_path                      Tokenizer vocabulary file.
      --do_lower_case                   Whether to convert upper case letters to
                                        lower case.
      --codepoint_level                 Whether to return character position in
                                        offsets.
      --sent_path                       Sentence data path to be processed.
      --num_threads                     Number of parallel threads.
      --batch_size                      Batch size.
```

```shell
./examples/cpp/speed_tests --vocab_path ../data/bert-base-chinese-vocab.txt --sent_path ../data/sents.txt --do_lower_case --num_threads 1 --batch_size 1
```

在 sents.txt (10098 条句子) 上的测试结果如下：

| batch_size    | 1     | 32    | 64    | 128   | 512   | 1024  |
| ------------- | ----- | ----- | ----- | ----- | ----- | ----- |
| num_threads=1 | 0.349 | 0.344 | 0.347 | 0.342 | 0.349 | 0.357 |
| num_threads=4 | —     | 0.243 | 0.223 | 0.213 | 0.180 | 0.170 |

在 sents_17w.txt (179608 条句子) 上的测试结果如下：

| batch_size    | 1     | 32    | 64    | 128   | 512   | 1024  |
| ------------- | ----- | ----- | ----- | ----- | ----- | ----- |
| num_threads=1 | 2.241 | 2.558 | 2.532 | 2.507 | 2.431 | 2.443 |
| num_threads=4 | —     | 2.691 | 2.610 | 2.338 | 1.791 | 1.472 |

## Python

### Requirements

- Python version >= 3.6

- pybind11 >= 2.2

- setuptools >= 0.7.0

### Installation

从 github 仓库中获取最新版本安装：

```shell
git clone https://github.com/zejunwang1/easytokenizer
cd easytokenizer/
python setup.py install
```

### Demo

示例位于 example/python/demo.py

```python
# coding=utf-8

from easytokenizer import AutoTokenizer

vocab_path = "../../data/bert-base-chinese-vocab.txt"
tokenizer = AutoTokenizer(vocab_path, do_lower_case = True)

# encode batch texts
texts = ["计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。",
         "清华大学的[MASK]算机科学与技术专业实力全国第一。"]
result = tokenizer.encode(
    texts, num_threads = 1, add_cls_sep = True, padding = True, padding_to_max_length = False,
    truncation = True, max_length = 512)
print("encode batch texts:")
print("input_ids:")
print(result["input_ids"])
print("attention_mask:")
print(result["attention_mask"])
print("offsets:")
print(result["offsets"])
```

运行后结果如下：

```
encode batch texts:
input_ids:
[[101, 6369, 5050, 3322, 4906, 2110, 680, 2825, 3318, 8020, 8134, 11300, 8196, 9982, 8256, 11061, 8021, 3221, 671, 7305, 3249, 6858, 7770, 5023, 2110, 3413, 3315, 4906, 683, 689, 511, 102], [101, 3926, 1290, 1920, 2110, 4638, 103, 5050, 3322, 4906, 2110, 680, 2825, 3318, 683, 689, 2141, 1213, 1059, 1744, 5018, 671, 511, 102, 0, 0, 0, 0, 0, 0, 0, 0]]
attention_mask:
[[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0]]
offsets:
[[0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 12, 12, 15, 15, 17, 18, 25, 26, 29, 30, 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 50, 50, 51, 51, 52, 52, 53, 53, 54, 54, 55], [0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27]]
```

### Speed

笔者比较了如下四个文本 Tokenizer 工具的处理速度：

- HuggingFace transformers 中基于 python 实现的 BertTokenizer

- HuggingFace transformers 中基于 tokenizers 库实现的 BertTokenizerFast

- paddlenlp 开源的 faster_tokenizer ( paddlenlp-2.4.0  faster-tokenizer-0.2.0 )

- 本项目实现的 easytokenizer

运行 python_testing/test_speed.py 进行速度测试：

```
usage: test_speed.py [-h] --vocab_path VOCAB_PATH --data_path DATA_PATH
                     [--num_threads NUM_THREADS] [--batch_size BATCH_SIZE]
                     [--do_lower_case]
```

```shell
python test_speed.py --vocab_path ../data/bert-base-chinese-vocab.txt --data_path ../data/sents.txt --do_lower_case --num_threads 1 --batch_size 1
```

分别实验了 batch_size=1, 32, 64, 128, 512, 1024，不同工具在 sents.txt (10098 条句子) 上的处理速度如下表所示：

| batch_size                                    | 1      | 32     | 64     | 128    | 512    | 1024   |
|:---------------------------------------------:|:------:|:------:|:------:|:------:|:------:|:------:|
| BertTokenizer                                 | 13.142 | 12.124 | 12.321 | 12.522 | 12.454 | 12.679 |
| BertTokenizerFast                             | 4.721  | 1.365  | 1.188  | 1.360  | 1.231  | 1.297  |
| paddlenlp-FasterTokenizer (OMP_NUM_THREADS=1) | 3.402  | 2.628  | 2.637  | 2.653  | 2.850  | 2.947  |
| paddlenlp-FasterTokenizer (OMP_NUM_THREADS=4) | —      | 1.312  | 1.271  | 1.315  | 1.473  | 1.553  |
| easytokenizer (num_threads=1)                 | 0.466  | 0.522  | 0.488  | 0.452  | 0.425  | 0.445  |
| easytokenizer (num_threads=4)                 | —      | 0.443  | 0.376  | 0.220  | 0.252  | 0.213  |

在 sents_17w.txt (179608 条句子) 上的测试结果如下：

| batch_size                                    | 1       | 32      | 64      | 128     | 512     | 1024    |
|:---------------------------------------------:|:-------:|:-------:|:-------:|:-------:|:-------:|:-------:|
| BertTokenizer                                 | 128.097 | 115.988 | 113.817 | 115.690 | 116.672 | 115.622 |
| BertTokenizerFast                             | 49.610  | 15.609  | 14.253  | 14.587  | 17.096  | 19.825  |
| paddlenlp-FasterTokenizer (OMP_NUM_THREADS=1) | 41.160  | 37.597  | 36.285  | 38.918  | 40.626  | 39.269  |
| paddlenlp-FasterTokenizer (OMP_NUM_THREADS=4) | —       | 16.383  | 15.863  | 15.852  | 20.339  | 22.570  |
| easytokenizer (num_threads=1)                 | 4.896   | 5.156   | 5.610   | 6.135   | 5.605   | 5.730   |
| easytokenizer (num_threads=4)                 | —       | 5.033   | 5.419   | 6.013   | 3.354   | 3.458   |

可以看出，easytokenizer 的处理速度显著超过其他工具。当 batch_size=1 时，单线程 (num_threads=1) 下的 easytokenizer 处理速度是 BertTokenizer 的 20 倍以上，是 BertTokenizerFast 和 paddlenlp-FasterTokenizer 的 7 倍以上。

当 batch_size>=32 时，由于 tokenizers 库优秀的多线程性能，BertTokenizerFast 的处理速度显著提升，4 线程下的 paddlenlp-FasterTokenizer 与 BertTokenizerFast 性能接近，但它们仍低于单线程下的 easytokenizer。当使用 easytokenizer 的多线程并行处理时，建议文本批处理大小在 128 以上。

## Links

- https://github.com/huggingface/transformers

- https://github.com/huggingface/tokenizers

- https://github.com/PaddlePaddle/PaddleNLP/tree/develop/fast_tokenizer

## Contact

邮箱： [wangzejunscut@126.com](mailto:wangzejunscut@126.com)

微信：autonlp
