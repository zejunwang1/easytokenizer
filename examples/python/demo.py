# coding=utf-8

from easytokenizer import AutoTokenizer

vocab_path = "../../data/bert-base-chinese-vocab.txt"
tokenizer = AutoTokenizer(vocab_path, do_lower_case=True)

# encode single text
text = "计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。"
result = tokenizer.encode(text, add_cls_sep=True, truncation=True, max_length=512)
print("encode single text:")
print("input_ids:")
print(result["input_ids"])
print("attention_mask:")
print(result["attention_mask"])
print("offsets:")
print(result["offsets"])
print("")

# encode batch texts
texts = ["计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。",
         "清华大学的[MASK]算机科学与技术专业实力全国第一。"]
result = tokenizer.encode(
    texts, num_threads=1, add_cls_sep=True, padding=True, padding_to_max_length=False,
    truncation=True, max_length=512)
print("encode batch texts:")
print("input_ids:")
print(result["input_ids"])
print("attention_mask:")
print(result["attention_mask"])
print("offsets:")
print(result["offsets"])
