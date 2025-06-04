#!/usr/bin/env python3

import os
import sys
import datetime
import urllib.parse

# Path to the HTML template
TEMPLATE_PATH = "./www/cgi-bin/template.html"

# Generate HTTP headers
# print("Content-type: text/html\r\n\r\n")

# Read the HTML template
try:
    with open(TEMPLATE_PATH, "r") as file:
        html_template = file.read()
except FileNotFoundError:
    print("<h1>Error: Template file not found</h1>")
    sys.exit(1)

# Dynamic content
current_time = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
request_method = os.environ.get("REQUEST_METHOD", "Unknown")
query_string = os.environ.get("QUERY_STRING", "")
query_parameters = ""
post_data = ""

# Process query string
if query_string:
    params = urllib.parse.parse_qs(query_string)
    query_parameters = "<ul>"
    for key, values in params.items():
        for value in values:
            query_parameters += f"<li><strong>{key}</strong>: {value}</li>"
    query_parameters += "</ul>"

# Process POST data
if request_method == "POST":
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        post_data_raw = sys.stdin.read(content_length)
        post_data = "<p>" + post_data_raw + "</p>"
        parsed_post = urllib.parse.parse_qs(post_data_raw)
        post_data += "<ul>"
        for key, values in parsed_post.items():
            for value in values:
                post_data += f"<li><strong>{key}</strong>: {value}</li>"
        post_data += "</ul>"
    except Exception as e:
        post_data += f"<p>Error parsing POST data: {e}</p>"

# Replace placeholders in the template
html_output = html_template.replace("{{current_time}}", current_time)
html_output = html_output.replace("{{request_method}}", request_method)
html_output = html_output.replace("{{query_string}}", query_string)
html_output = html_output.replace("{{query_parameters}}", query_parameters)
html_output = html_output.replace("{{post_data}}", post_data)

# Add CSS link to the output
html_output = html_output.replace(
    "<head>",
    "<head>\n    <link rel=\"stylesheet\" type=\"text/css\" href=\"/style/style.css\">"
)

# Output the final HTML
print(html_output)
