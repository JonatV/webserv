#!/bin/bash

# Function to decode URL encoded strings
urldecode() {
    printf '%b' "${1//%/\\x}"
}

# Function to parse query string
parse_query_string() {
    if [ -n "$QUERY_STRING" ]; then
        # Split by & and then by =
        echo "$QUERY_STRING" | tr '&' '\n' | while IFS='=' read -r key value; do
            echo "$key=$(urldecode "$value")"
        done
    fi
}

# Function to parse POST data
parse_post_data() {
    if [ "$REQUEST_METHOD" = "POST" ] && [ -n "$CONTENT_LENGTH" ]; then
        # Read POST data from stdin
        POST_DATA=$(head -c "$CONTENT_LENGTH")
        echo "$POST_DATA" | tr '&' '\n' | while IFS='=' read -r key value; do
            echo "$key=$(urldecode "$value")"
        done
    fi
}

# Generate Star Wars themed response based on method
generate_starwars_response() {
    local method="$1"
    local action="$2"
    local message="$3"
    local username="$4"
    
    case "$method" in
        "POST")
            if [ -n "$username" ] && [ -n "$message" ]; then
                RESPONSE="<div class='jedi-message'>
                    <h3>🌟 Message reçu, jeune Padawan $username!</h3>
                    <p>Votre message: \"$message\"</p>
                    <p>La Force est forte en vous...</p>
                </div>"
            else
                RESPONSE="<div class='sith-warning'>
                    <h3>⚡ Données POST détectées!</h3>
                    <p>Le côté obscur de la Force révèle: $(echo "$POST_DATA" | head -c 100)</p>
                </div>"
            fi
            ;;
        "GET")
            case "$action" in
                "deathstar")
                    RESPONSE="<div class='deathstar-info'>
                        <h3>💀 Death Star Status</h3>
                        <p>🔴 OPERATIONAL</p>
                        <p>Target: Rebel Base</p>
                        <p>Time to fire: $(date)</p>
                    </div>"
                    ;;
                "rebels")
                    RESPONSE="<div class='rebel-info'>
                        <h3>🚀 Rebel Alliance Intel</h3>
                        <p>✅ Princess Leia: Safe</p>
                        <p>✅ Luke Skywalker: Training</p>
                        <p>✅ Han Solo: In carbonite</p>
                    </div>"
                    ;;
                "force")
                    RESPONSE="<div class='force-info'>
                        <h3>🌌 Force Level Check</h3>
                        <p>Midichlorian count: $((RANDOM % 20000 + 1000))</p>
                        <p>Force sensitivity: $( [ $((RANDOM % 2)) -eq 0 ] && echo "Strong" || echo "Weak" )</p>
                    </div>"
                    ;;
                *)
                    RESPONSE="<div class='default-info'>
                        <h3>📡 Transmission reçue</h3>
                        <p>Query parameters: $QUERY_STRING</p>
                    </div>"
                    ;;
            esac
            ;;
        *)
            RESPONSE="<div class='welcome-message'>
                <h3>🌟 Welcome to the Galaxy</h3>
                <p>Ready to receive your commands, Master.</p>
            </div>"
            ;;
    esac
}

# Main execution starts here
echo "Content-Type: text/html"
echo ""

# Parse parameters
if [ "$REQUEST_METHOD" = "POST" ]; then
    POST_PARAMS=$(parse_post_data)
    username=$(echo "$POST_PARAMS" | grep "^username=" | cut -d'=' -f2-)
    message=$(echo "$POST_PARAMS" | grep "^message=" | cut -d'=' -f2-)
fi

if [ -n "$QUERY_STRING" ]; then
    GET_PARAMS=$(parse_query_string)
    action=$(echo "$QUERY_STRING" | sed -n 's/.*action=\([^&]*\).*/\1/p')
fi

# Generate appropriate response
generate_starwars_response "$REQUEST_METHOD" "$action" "$message" "$username"

# Output the HTML
cat << EOF
<!DOCTYPE html>
<html>
<head>
    <title>Star Wars CGI Command Center</title>
    <link rel="stylesheet" type="text/css" href="/style/style.css">
    <style>
        body {
            background: linear-gradient(45deg, #000428, #004e92);
            color: #FFE81F;
            font-family: 'Courier New', monospace;
            margin: 0;
            padding: 20px;
        }
        .container {
            max-width: 900px;
            margin: 0 auto;
            background: rgba(0, 0, 0, 0.8);
            border: 2px solid #FFE81F;
            border-radius: 10px;
            padding: 20px;
        }
        .method-section {
            background: rgba(255, 232, 31, 0.1);
            border: 1px solid #FFE81F;
            border-radius: 8px;
            padding: 20px;
            margin: 20px 0;
        }
        .jedi-message {
            background: linear-gradient(45deg, #004e92, #000428);
            border-left: 5px solid #00FF00;
            padding: 15px;
            color: #00FF00;
        }
        .sith-warning {
            background: linear-gradient(45deg, #8B0000, #FF0000);
            border-left: 5px solid #FF0000;
            padding: 15px;
            color: #FFE81F;
        }
        .deathstar-info, .rebel-info, .force-info, .default-info {
            background: rgba(255, 232, 31, 0.2);
            border: 1px solid #FFE81F;
            padding: 15px;
            border-radius: 5px;
        }
        .get-links {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
            margin: 15px 0;
        }
        .get-links a {
            background: #FF6B35;
            color: white;
            padding: 8px 15px;
            text-decoration: none;
            border-radius: 5px;
            transition: all 0.3s;
            border: 2px solid transparent;
        }
        .get-links a:hover {
            background: transparent;
            border-color: #FF6B35;
            color: #FF6B35;
        }
        input, textarea {
            width: 100%;
            max-width: 400px;
            padding: 10px;
            background: rgba(0, 0, 0, 0.7);
            border: 1px solid #FFE81F;
            color: #FFE81F;
            border-radius: 5px;
            font-family: 'Courier New', monospace;
        }
        .btn {
            background: transparent;
            border: 2px solid #FFE81F;
            color: #FFE81F;
            padding: 10px 20px;
            cursor: pointer;
            border-radius: 5px;
            font-family: 'Courier New', monospace;
            transition: all 0.3s;
        }
        .btn:hover {
            background: #FFE81F;
            color: #000;
        }
        .debug-panel {
            background: rgba(0, 0, 0, 0.9);
            border: 1px solid #666;
            padding: 15px;
            margin: 20px 0;
            border-radius: 5px;
            font-size: 0.9em;
            color: #ccc;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌌 Star Wars CGI Command Center 🌌</h1>
        <h2>Imperial Shell Interface</h2>
        
        $RESPONSE
        
        <div class="method-section">
            <h3>📝 POST Mission Report</h3>
            <p>Send a message to the Rebellion:</p>
            <form method="POST" action="">
                <div style="margin: 10px 0;">
                    <label>Rebel Code Name:</label><br>
                    <input type="text" name="username" placeholder="Enter your code name" required>
                </div>
                <div style="margin: 10px 0;">
                    <label>Secret Message:</label><br>
                    <textarea name="message" rows="3" placeholder="Your message to the Alliance..." required></textarea>
                </div>
                <button type="submit" class="btn">🚀 Transmit Message</button>
            </form>
        </div>
        
        <div class="method-section">
            <h3>🔗 GET Intelligence Reports</h3>
            <p>Access Imperial databases:</p>
            <div class="get-links">
                <a href="?action=deathstar">💀 Death Star Status</a>
                <a href="?action=rebels">🚀 Rebel Intel</a>
                <a href="?action=force">🌌 Force Check</a>
                <a href="?planet=tatooine&sector=outer">🏜️ Tatooine Data</a>
            </div>
        </div>
        
        <div class="debug-panel">
            <h4>🤖 System Diagnostics</h4>
            <p><strong>Method:</strong> $REQUEST_METHOD</p>
            <p><strong>Query String:</strong> ${QUERY_STRING:-"None"}</p>
            <p><strong>Content Length:</strong> ${CONTENT_LENGTH:-"0"}</p>
            <p><strong>Server Time:</strong> $(date)</p>
            <p><strong>Server Port:</strong> ${SERVER_PORT:-"Unknown"}</p>
            <p><strong>Script Name:</strong> ${SCRIPT_NAME:-"Unknown"}</p>
        </div>
    </div>
</body>
</html>
EOF