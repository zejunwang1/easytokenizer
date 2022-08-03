easytokenizer: 一个简单高效的字词切分工具
=========================================

easytokenizer 是一个简单高效的字词切分工具，支持类似 HuggingFace
transformers 包中BertTokenizer 的词语切分和标记化功能。具有如下特点：

-  实现高效，基于双数组字典树 (double-array trie) 和 Unicode 规范化工具
   utf8proc

-  多线程，处理速度快

-  支持 c++ 和 python

C++
---

使用示例参考 demo.cc，通过 cmake 进行编译：

.. code:: shell

   git clone https://github.com/zejunwang1/easytokenizer
   cd easytokenizer/
   mkdir build
   cd build/
   cmake ..
   make -j4

执行上述命令后，会在 build 文件夹下生成可执行文件 demo

.. code:: shell

   ./demo -vocab ../vocab.txt

运行后部分结果如下：

::

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

Python
------

Requirements
~~~~~~~~~~~~

-  Python version >= 3.6

-  pybind11 >= 2.2

-  setuptools >= 0.7.0

Installation
~~~~~~~~~~~~

通过 pip 命令直接安装：

.. code:: shell

   pip install easytokenizer

或者从 github 仓库中获取最新版本安装：

.. code:: shell

   git clone https://github.com/zejunwang1/easytokenizer
   cd easytokenizer/
   python setup.py install

Demo
~~~~

.. code:: python

   # coding=utf-8

   from easytokenizer import AutoTokenizer

   vocab_path = "vocab.txt"
   tokenizer = AutoTokenizer(vocab_path, do_lower_case = True)

   # encode batch texts
   texts = ["计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。",
            "清华大学的[MASK]算机科学与技术专业实力全国第一。"]
   input_ids, token_type_ids, attention_mask, offsets = tokenizer.encode(
       texts, num_threads = 1, add_cls_sep = True, padding = True, padding_to_max_length = False,
       truncation = True, max_length = 512)
   print("encode batch texts:")
   print("input_ids:")
   print(input_ids)
   print("attention_mask:")
   print(attention_mask)
   print("offsets:")
   print(offsets)

运行后结果如下：

::

   encode batch texts:
   input_ids:
   [[101, 6369, 5050, 3322, 4906, 2110, 680, 2825, 3318, 8020, 8134, 11300, 8196, 9982, 8256, 11061, 8021, 3221, 671, 7305, 3249, 6858, 7770, 5023, 2110, 3413, 3315, 4906, 683, 689, 511, 102], [101, 3926, 1290, 1920, 2110, 4638, 103, 5050, 3322, 4906, 2110, 680, 2825, 3318, 683, 689, 2141, 1213, 1059, 1744, 5018, 671, 511, 102, 0, 0, 0, 0, 0, 0, 0, 0]]
   attention_mask:
   [[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0]]
   offsets:
   [[(0, 3), (3, 3), (6, 3), (9, 3), (12, 3), (15, 3), (18, 3), (21, 3), (24, 3), (27, 3), (30, 3), (33, 2), (36, 7), (44, 3), (48, 10), (58, 3), (61, 3), (64, 3), (67, 3), (70, 3), (73, 3), (76, 3), (79, 3), (82, 3), (85, 3), (88, 3), (91, 3), (94, 3), (97, 3), (100, 3)], [(0, 3), (3, 3), (6, 3), (9, 3), (12, 3), (15, 6), (21, 3), (24, 3), (27, 3), (30, 3), (33, 3), (36, 3), (39, 3), (42, 3), (45, 3), (48, 3), (51, 3), (54, 3), (57, 3), (60, 3), (63, 3), (66, 3)]]

Speed
~~~~~

笔者比较了如下四个 tokenization 工具的处理速度：

-  HuggingFace transformers 中基于 python 实现的 BertTokenizer

-  HuggingFace transformers 中基于 tokenizers 库实现的 BertTokenizerFast

-  paddlenlp 开源的 faster_tokenizer

-  本项目中实现的 easytokenizer

在 tests 文件夹中包含测试需要用到的句子文件 sents.txt，sents.txt
为从中文维基百科中抽取的 10098 条句子（平均长度在 128 个字符以上），运行
test_speed.py 进行速度测试：

::

   usage: test_speed.py [-h] --vocab_path VOCAB_PATH --data_path DATA_PATH
                        [--num_threads NUM_THREADS] [--batch_size BATCH_SIZE]

.. code:: shell

   python test_speed.py --vocab_path ../vocab.txt --data_path sents.txt --num_threads 4 --batch_size 64

分别实验了 batch_size=1, 8, 16, 32, 64,
128，不同工具的处理速度如下表所示：

+-------------------------------+--------+--------+--------+--------+--------+--------+
| batch_size                    | 1      | 8      | 16     | 32     | 64     | 128    |
+===============================+========+========+========+========+========+========+
| BertTokenizer                 | 11.383 | 10.823 | 10.847 | 10.498 | 10.539 | 10.834 |
+-------------------------------+--------+--------+--------+--------+--------+--------+
| BertTokenizerFast             | 4.016  | 1.860  | 1.405  | 1.352  | 1.181  | 1.139  |
+-------------------------------+--------+--------+--------+--------+--------+--------+
| paddlenlp-FasterTokenizer     | 2.861  | 2.342  | 2.258  | 2.216  | 2.344  | 2.194  |
+-------------------------------+--------+--------+--------+--------+--------+--------+
| easytokenizer (num_threads=1) | 2.701  | 1.890  | 1.754  | 1.563  | 1.871  | 2.089  |
+-------------------------------+--------+--------+--------+--------+--------+--------+
| easytokenizer (num_threads=4) | 3.405  | 0.803  | 0.790  | 0.768  | 0.861  | 0.824  |
+-------------------------------+--------+--------+--------+--------+--------+--------+

可以看出，当 batch_size=1 时，单线程 (num_threads=1) 下的 easytokenizer
处理速度最快；当 batch_size>=8 时，四线程 (num_threads=4) 下的
easytokenizer 处理速度最快。

Contact
-------

邮箱： \ wangzejunscut@126.com

微信：autonlp
