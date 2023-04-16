# Ehsan Neural Network

To use `enn` first you need to create enn samples using `BaTool` in the `enn` mode. to do that run the following command
```
BaTool e
```

ENN works in 4 different modes that can be selected with command line options. Here is the usage help

```
enn <option>
```

### 1. Test Mode (t)
Quick verification test.

Check single sample against single model

if `input_name` and `model_name` be the same you should see `O0` to be `0` and `O1` to be close to `1`

### 2. Test Full Mode (tf)  
Test all models(all words in the word_list) against false data.

In the perfect world all O0 should be 1, but if not it will output how many false positive(wrong) the system generate

In this mode enn will write one line per model in the output

### 3. File Mode (f)
Only for internal `enn` development.

### 4. Learn Mode (learning rate, <num>)
  
Usage example:
  
```
enn 0.01
```
