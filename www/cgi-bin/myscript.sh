#!/bin/sh

# Generate HTTP headers
echo "Content-type: text/html"
echo ""

# Generate HTML content
echo "<!DOCTYPE html>"
echo "<html>"
echo "<head>"
echo "    <title>Shell CGI Script</title>"
echo "    <link rel=\"stylesheet\" type=\"text/css\" href=\"/style/style.css\">"
echo "</head>"
echo "<body>"
echo "    <h1>Hello from Shell CGI</h1>"
echo "    <p><strong>Server time:</strong> $(date)</p>"
echo "    <p><strong>Request method:</strong> $REQUEST_METHOD</p>"
echo "    <p><strong>Query string:</strong> $QUERY_STRING</p>"

# Parse query string
if [ ! -z "$QUERY_STRING" ]; then
    echo "    <h3>Query Parameters:</h3>"
    echo "    <ul>"
    for param in $(echo "$QUERY_STRING" | tr '&' '\n'); do
        key=$(echo "$param" | cut -d '=' -f 1)
        value=$(echo "$param" | cut -d '=' -f 2)
        echo "        <li><strong>$key</strong>: $value</li>"
    done
    echo "    </ul>"
fi

echo "</body>"
echo "</html>"