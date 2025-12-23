#! /usr/bin/python3

import tkinter as tk
from tkinter import ttk
import urllib.parse
import base64
import html
import pyperclip

def encode_payload(payload: str, method: str) -> str:
    method = method.lower()

    if method == "url":
        return urllib.parse.quote(payload)

    elif method == "double_url":
        return urllib.parse.quote(urllib.parse.quote(payload))

    elif method == "html":
        return html.escape(payload)

    elif method == "html_hex":
        return ''.join(f'&#x{ord(c):X};' for c in payload)

    elif method == "html_dec":
        return ''.join(f'&#{ord(c)};' for c in payload)

    elif method == "base64":
        return base64.b64encode(payload.encode()).decode()

    elif method == "hex":
        return ''.join(f'\\x{ord(c):02x}' for c in payload)

    elif method == "octal":
        return ''.join(f'\\{oct(ord(c))[2:]}' for c in payload)

    elif method == "rot13":
        return payload.translate(str.maketrans(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
            "NOPQRSTUVWXYZABCDEFGHIJKLMnopqrstuvwxyzabcdefghijklm"))

    elif method == "mixed_case":
        return ''.join(c.upper() if i % 2 else c.lower() for i, c in enumerate(payload))
    
    elif method == "unicode":
        return ''.join(f'\\u{ord(c):04x}' for c in payload)

    else:
        return "Unknown encoding method."

def on_encode():
    payload = input_field.get()
    method = method_selector.get()
    result = encode_payload(payload, method)
    output_field.config(state='normal')
    output_field.delete(1.0, tk.END)
    output_field.insert(tk.END, result)
    output_field.config(state='disabled')
    pyperclip.copy(result)

# GUI setup
root = tk.Tk()
root.title("Encod3r")

# Main frame
main_frame = tk.Frame(root, padx=20, pady=20)
main_frame.pack(fill="both", expand=True)

# Input field
tk.Label(main_frame, text="Input String:").pack(anchor='w')
input_field = tk.Entry(main_frame, width=50)
input_field.pack(pady=5)
input_field.focus_set()

# Encoding selector
tk.Label(main_frame, text="Encoding Method:").pack(anchor='w')
methods = ["url", "double_url", "html", "html_hex", "html_dec", "base64", "hex", "octal", "rot13", "mixed_case", "unicode"]
method_selector = ttk.Combobox(main_frame, values=methods, state="readonly")
method_selector.set("url")
method_selector.pack(pady=5)

# Encode button
encode_button = tk.Button(main_frame, text="Encode", command=on_encode)
encode_button.pack(pady=5)

# Bind the Return key to the Encode button
root.bind('<Return>', lambda event: on_encode())

# Output field
tk.Label(main_frame, text="Encoded Output:").pack(anchor='w')
output_field = tk.Text(main_frame, height=4, width=50, state='disabled')
output_field.pack(pady=5)

root.mainloop()
