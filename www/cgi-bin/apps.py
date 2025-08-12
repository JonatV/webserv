#!/usr/bin/env python3

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

def generate_interactive_form():
    """Génère un formulaire interactif qui gère GET et POST"""
    
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
            result_message = f"Message reçu de {escape(username)}: {escape(message)}"
        else:
            result_message = "Données POST reçues: " + str(post_data)
    
    elif method == "GET" and get_params:
        if "action" in get_params:
            action = get_params["action"][0]
            if action == "time":
                result_message = f"Heure actuelle: {datetime.datetime.now().strftime('%H:%M:%S')}"
            elif action == "info":
                result_message = f"Informations serveur: Python CGI sur port {os.environ.get('SERVER_PORT', 'unknown')}"
            else:
                result_message = f"Action inconnue: {escape(action)}"
    
    html = f"""
    <!DOCTYPE html>
    <html>
    <head>
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
            .form-group {{
                margin: 15px 0;
            }}
            label {{
                display: block;
                margin-bottom: 5px;
                color: #e5e1dc;
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
            .result {{
                background: #1a3d1a;
                border: 1px solid #4a7c4a;
                padding: 15px;
                border-radius: 5px;
                margin: 20px 0;
                color: #90ee90;
            }}
            .debug-info {{
                background: #1a1a3d;
                border: 1px solid #4a4a7c;
                padding: 15px;
                border-radius: 5px;
                margin: 20px 0;
                color: #b0b0ff;
                font-family: monospace;
                font-size: 0.9em;
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
            .get-links a:hover {{
                background: #b83d45;
            }}
        </style>
    </head>
    <body>
        <div class="container">
            <h1>🚀 CGI Interactive Demo</h1>
            
            {"<div class='result'>" + result_message + "</div>" if result_message else ""}
            
            <div class="method-section">
                <h2>📝 POST Method - Formulaire</h2>
                <p>Envoie des données via POST:</p>
                <form method="POST" action="">
                    <div class="form-group">
                        <label for="username">Nom d'utilisateur:</label>
                        <input type="text" id="username" name="username" placeholder="Ton pseudo" required>
                    </div>
                    <div class="form-group">
                        <label for="message">Message:</label>
                        <textarea id="message" name="message" rows="4" placeholder="Ton message ici..." required></textarea>
                    </div>
                    <button type="submit" class="bigBtn">Envoyer via POST</button>
                </form>
            </div>
            
            <div class="method-section">
                <h2>🔗 GET Method - Liens</h2>
                <p>Clique sur les liens pour tester GET:</p>
                <div class="get-links">
                    <a href="?action=time">Obtenir l'heure</a>
                    <a href="?action=info">Info serveur</a>
                    <a href="?action=custom&param=hello">Paramètre custom</a>
                    <a href="?name=John&age=25">Params multiples</a>
                </div>
            </div>
            
            <div class="debug-info">
                <h3>🐛 Debug Info</h3>
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
    </html>
    """
    return html

def generate_api_demo():
    """Génère une API-like demo avec JSON response"""
    method = os.environ.get("REQUEST_METHOD", "GET")
    
    if method == "POST":
        post_data = parse_post_data()
        response = {
            "status": "success",
            "method": "POST",
            "data_received": post_data,
            "timestamp": datetime.datetime.now().isoformat()
        }
        print("Content-Type: application/json\\n")
        print(json.dumps(response, indent=2))
        return
    
    else:  # GET
        get_params = parse_get_params()
        response = {
            "status": "success",
            "method": "GET",
            "parameters": get_params,
            "server_info": {
                "script_name": os.environ.get("SCRIPT_NAME", ""),
                "server_name": os.environ.get("SERVER_NAME", ""),
                "server_port": os.environ.get("SERVER_PORT", ""),
            },
            "timestamp": datetime.datetime.now().isoformat()
        }
        print("Content-Type: application/json\\n")
        print(json.dumps(response, indent=2))
        return

def generate_file_upload_demo():
    """Démo d'upload de fichier (simulation)"""
    method = os.environ.get("REQUEST_METHOD", "GET")
    
    if method == "POST":
        # Pour un vrai upload, il faudrait parser multipart/form-data
        # Ici on simule juste
        post_data = parse_post_data()
        message = "Upload simulé - données reçues!"
    else:
        message = ""
    
    html = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>File Upload Demo - CGI</title>
        <link rel="stylesheet" type="text/css" href="/style/style.css">
    </head>
    <body>
        <div class="container" style="max-width: 600px; margin: 0 auto; padding: 20px;">
            <h1>📁 File Upload Demo</h1>
            
            {"<div class='result'>" + message + "</div>" if message else ""}
            
            <form method="POST" enctype="multipart/form-data" action="">
                <div style="margin: 20px 0;">
                    <label>Fichier à uploader:</label>
                    <input type="file" name="uploaded_file" accept=".txt,.py,.sh">
                </div>
                <div style="margin: 20px 0;">
                    <label>Description:</label>
                    <input type="text" name="description" placeholder="Description du fichier">
                </div>
                <button type="submit" class="bigBtn">Uploader</button>
            </form>
            
            <p><em>Note: Ceci est une démo. Dans un vrai serveur, il faudrait parser multipart/form-data.</em></p>
        </div>
    </body>
    </html>
    """
    return html

# Main execution
def main():
    # Choix de l'application selon un paramètre ou aléatoire
    query = os.environ.get("QUERY_STRING", "")
    params = urllib.parse.parse_qs(query)
    app_type = params.get('app', ['form'])[0]
    
    if app_type == 'api':
        generate_api_demo()
    elif app_type == 'upload':
        print("Content-Type: text/html\\n")
        print(generate_file_upload_demo())
    else:  # 'form' par défaut
        print("Content-Type: text/html\\n")
        print(generate_interactive_form())

if __name__ == "__main__":
    main()