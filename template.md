# <font  color='3d8c95'>H1</font>

## <font  color='dc843f'>H2</font>

<font  color='fed3a8'>*h*</font>

<font  color='235977'>RESERVE</font>

[atomicQueue](https://github.com/max0x7ba/atomic_queue?tab=readme-ov-file)

![alt text](XXX.png)

PROJECT_ID=XXX
echo "==================${PROJECT_ID}======================"
g++ -o ${PROJECT_ID}.elf ${PROJECT_ID}.cpp -std=c++2a -latomic -lpthread -march=native -msse2 -O3 -DNDEBUG # -mavx2
if [ $? -eq 0 ]; then
    ./${PROJECT_ID}.elf
fi

| 对比项 | 优点 | 缺点 | 选择建议 |
| ------ | ---- | ---- | -------- |
|        |      |      |          |

---
keybindings.json
```
// Place your key bindings in this file to override the defaults
[
    {
        "key": "f6",
        "command": "editor.action.insertSnippet",
        "args": {
          "snippet": "# <font color=\"3d8c95\">${TM_SELECTED_TEXT}</font>$0"
        },
        "when": "editorTextFocus && editorHasSelection"
    },
    {
        "key": "f7",
        "command": "editor.action.insertSnippet",
        "args": {
          "snippet": "## <font color=\"dc843f\">${TM_SELECTED_TEXT}</font>$0"
        },
        "when": "editorTextFocus && editorHasSelection"
    },
    {
        "key": "f8",
        "command": "editor.action.insertSnippet",
        "args": {
          "snippet": "<font color=\"fed3a8\">${TM_SELECTED_TEXT}</font>$0"
        },
        "when": "editorTextFocus && editorHasSelection"
    }
]
```

---
web mirror

create
```
wget --mirror --convert-links --adjust-extension --page-requisites --no-parent https://martin.ankerl.com/2022/08/27/hashmap-bench-01/
```
browse
1. vscode install LiveServer
2. right click `index.html` -> Open with Live Server