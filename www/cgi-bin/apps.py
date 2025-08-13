#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import json
import datetime
import urllib.parse
from html import escape

def parse_get_params():
    """Parse les paramètres GET depuis QUERY_STRING"""
    query_string = os.environ.get("QUERY_STRING", "")
    if not query_string:
        return {}
    return urllib.parse.parse_qs(query_string)

def parse_post_data():
    """Parse les données POST depuis stdin"""
    request_method = os.environ.get("REQUEST_METHOD", "")
    if request_method != "POST":
        return {}
    
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        if content_length == 0:
            return {}
        
        # Lire les données POST
        post_data = sys.stdin.read(content_length)
        
        # Parser selon le Content-Type
        content_type = os.environ.get("CONTENT_TYPE", "")
        
        if "application/x-www-form-urlencoded" in content_type:
            return urllib.parse.parse_qs(post_data)
        elif "application/json" in content_type:
            return json.loads(post_data)
        else:
            # Raw data
            return {"raw_data": post_data}
    except Exception as e:
        return {"error": str(e)}

def main():
    # Récupérer les données GET et POST
    get_params = parse_get_params()
    post_data = parse_post_data()
    method = os.environ.get("REQUEST_METHOD", "GET")
    
    # Traitement des données selon la méthode
    result_message = ""
    if method == "POST":
        if "username" in post_data and "message" in post_data:
            username = post_data["username"][0] if isinstance(post_data["username"], list) else post_data["username"]
            message = post_data["message"][0] if isinstance(post_data["message"], list) else post_data["message"]
            result_message = f"Message recu de {escape(username)}: {escape(message)}"
        else:
            result_message = "Donnees POST recues: " + str(post_data)
    
    elif method == "GET" and get_params:
        if "action" in get_params:
            action = get_params["action"][0]
            if action == "time":
                result_message = f"Heure actuelle: {datetime.datetime.now().strftime('%H:%M:%S')}"
            elif action == "info":
                result_message = f"Informations serveur: Python CGI sur port {os.environ.get('SERVER_PORT', 'unknown')}"
            else:
                result_message = f"Action inconnue: {escape(action)}"

    # Headers avec encodage UTF-8
    print("Content-Type: text/html; charset=utf-8")
    print("")

    # HTML avec emojis simples
    html = f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Interactive CGI - GET/POST Demo</title>
    <link rel="stylesheet" type="text/css" href="/style/style.css">
    <style>
        .container {{
            max-width: 800px;
            margin: 20px auto;
            padding: 20px;
        }}
        .method-section {{
            background: #2f2f2f;
            border: 1px solid #575757;
            border-radius: 8px;
            padding: 20px;
            margin: 20px 0;
        }}
        .result {{
            background: #1a3d1a;
            border: 1px solid #4a7c4a;
            padding: 15px;
            border-radius: 5px;
            margin: 20px 0;
            color: #90ee90;
        }}
        .get-links {{
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
        }}
        .get-links a {{
            background: #df454e;
            color: white;
            padding: 8px 15px;
            text-decoration: none;
            border-radius: 5px;
            transition: background 0.3s;
        }}
        input, textarea {{
            width: 100%;
            max-width: 400px;
            padding: 10px;
            background: #202020;
            border: 1px solid #575757;
            color: #e5e1dc;
            border-radius: 5px;
            font-family: monospace;
        }}
        .bigBtn {{
            background: transparent;
            border: 2px solid #e5e1dc;
            color: #e5e1dc;
            padding: 10px 20px;
            cursor: pointer;
            border-radius: 5px;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Python CGI Interactive Demo</h1>
        
        {"<div class='result'>" + result_message + "</div>" if result_message else ""}
        
        <div class="method-section">
            <h2>POST Method - Formulaire</h2>
            <p>Envoie des donnees via POST:</p>
            <form method="POST" action="">
                <div style="margin: 15px 0;">
                    <label for="username">Nom d'utilisateur:</label>
                    <input type="text" id="username" name="username" placeholder="Ton pseudo" required>
                </div>
                <div style="margin: 15px 0;">
                    <label for="message">Message:</label>
                    <textarea id="message" name="message" rows="4" placeholder="Ton message ici..." required></textarea>
                </div>
                <button type="submit" class="bigBtn">Envoyer via POST</button>
            </form>
        </div>
        
        <div class="method-section">
            <h2>GET Method - Liens</h2>
            <p>Clique sur les liens pour tester GET:</p>
            <div class="get-links">
                <a href="?action=time">Obtenir l'heure</a>
                <a href="?action=info">Info serveur</a>
                <a href="?action=custom&param=hello">Parametre custom</a>
                <a href="?name=John&age=25">Params multiples</a>
            </div>
        </div>
        
        <div style="background: #1a1a3d; border: 1px solid #4a4a7c; padding: 15px; border-radius: 5px; margin: 20px 0; color: #b0b0ff; font-family: monospace; font-size: 0.9em;">
            <h3>Debug Info</h3>
            <p><strong>Method:</strong> {method}</p>
            <p><strong>Query String:</strong> {os.environ.get("QUERY_STRING", "None")}</p>
            <p><strong>Content Type:</strong> {os.environ.get("CONTENT_TYPE", "None")}</p>
            <p><strong>Content Length:</strong> {os.environ.get("CONTENT_LENGTH", "0")}</p>
            <p><strong>GET Params:</strong> {escape(str(get_params)) if get_params else "None"}</p>
            <p><strong>POST Data:</strong> {escape(str(post_data)) if post_data else "None"}</p>
            <p><strong>Timestamp:</strong> {datetime.datetime.now()}</p>
        </div>
    </div>
</body>
</html>"""
    
    print(html)

if __name__ == "__main__":
    main()