# Other Components
There are more components you can use.

![others](https://user-images.githubusercontent.com/69258547/171445898-7b696c95-a081-4a3a-aa66-71b8265f64d8.png)

```
"components": [
    {
        "type": "static_text",
        "label": "This is a sample GUI. Edit 'gui_definition.json' for your scripts."
    },
    {
        "type": "choice",
        "label": "num",
        "items": ["one", "two", "three"],
        "values": ["1", "2", "3"],
        "default": 1
    },
    {
        "type": "check",
        "label": "checkbox",
        "value": "true"
    },
    {
        "type": "text_box",
        "label": "Some text"
    }
]
```

`static_text` is a string component. You don't need to put `%*%` in command for this component.<br>
`choice` is a combo box. `values` will be inputted to command.<br>
`check` is a check box. `value` will be inputted to command if you check the check box.<br>
`text_box` is a text box.
