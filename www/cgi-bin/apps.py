#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import datetime
import urllib.parse
from html import escape
import random

def get_elvish_quote():
    """Retourne une citation elfique/fantasy (inventée)"""
    quotes = [
        "Mellon - L'amitié est la plus grande des magies",
        "Elen síla lúmenn' omentielvo - Que les étoiles brillent sur notre rencontre", 
        "Mae govannen - Bien rencontré, voyageur",
        "Gurth an Glamhoth - La mort aux orcs !",
        "Le annon hen - Je vais là-bas",
        "Namárië - Adieu, que le voyage soit sûr"
    ]
    return random.choice(quotes)

def get_character_icon(character):
    """Retourne l'icône et le titre pour chaque personnage"""
    icons = {
        "gandalf": ("🧙‍♂️", "Le Gris"),
        "legolas": ("🏹", "Prince Elfe"),
        "aragorn": ("⚔️", "Roi Elessar"),
        "gimli": ("🪓", "Fils de Glóin"),
        "frodo": ("💍", "Porteur"),
        "galadriel": ("✨", "Dame de Lumière"),
        "sauron": ("👁️", "Œil Rouge"),
        "saruman": ("🌪️", "Le Blanc")
    }
    return icons.get(character, ("🗡️", "Inconnu"))

def get_realm_icon(realm):
    """Retourne l'icône pour chaque royaume"""
    icons = {
        "gondor": "🏰",
        "rohan": "🐎", 
        "lothlorien": "🌳",
        "mordor": "🌋"
    }
    return icons.get(realm, "🗺️")

def get_character_info(character):
    """Base de données des personnages de la Terre du Milieu"""
    characters = {
        "gandalf": "Gandalf le Gris|Istar (Magicien)|Valinor → Terre du Milieu|Bâton et Épée|Défenseur de la Terre du Milieu|Guide de la Compagnie",
        "legolas": "Legolas|Prince Elfe|Royaume Sylvestre|Arc et Couteaux Elfiques|Archer légendaire|Membre de la Communauté",
        "aragorn": "Aragorn|Rôdeur du Nord|Gondor|Andúril (Épée)|Roi d'Arnor et Gondor|Héritier d'Isildur",
        "gimli": "Gimli|Guerrier Nain|Monts Brumeux|Hache de Guerre|Seigneur des Cavernes Scintillantes|Ami de Legolas",
        "frodo": "Frodon Sacquet|Hobbit|Comté|Sting (Dard)|Porteur de l'Anneau|Sauveur de la Terre du Milieu",
        "galadriel": "Galadriel|Dame Elfe|Lothlórien|Nenya (Anneau Elfique)|Dame de la Lumière|Gardienne de Lothlórien",
        "sauron": "Sauron|Seigneur Ténébreux|Mordor|Anneau Unique|Maître des Ténèbres|Ennemi de la Lumière",
        "saruman": "Saroumane le Blanc|Istar Corrompu|Isengard|Magie Noire|Traître du Conseil Blanc|Allié de Sauron"
    }
    return characters.get(character, "Être Inconnu|Race: Mystérieuse|Terre du Milieu|Arme: Inconnue|Destin: À écrire|Statut: En quête")

def get_realm_info(realm):
    """Informations sur les royaumes de la Terre du Milieu"""
    realms = {
        "gondor": {
            "name": "GONDOR",
            "type": "Royaume des Hommes",
            "ruler": "Roi Elessar (Aragorn)",
            "capital": "Minas Tirith",
            "population": "~500,000 habitants",
            "status": "Royaume restauré",
            "defenses": "Garde de la Citadelle, Tours de garde",
            "note": "Le plus puissant royaume des Hommes du Sud"
        },
        "rohan": {
            "name": "ROHAN",
            "type": "Royaume des Cavaliers",
            "ruler": "Roi Éomer",
            "capital": "Edoras",
            "population": "~200,000 habitants", 
            "status": "Allié du Gondor",
            "defenses": "Cavalerie de Rohan, Rohirrim",
            "note": "Maîtres des chevaux et de la cavalerie"
        },
        "lothlorien": {
            "name": "LOTHLÓRIEN",
            "type": "Royaume Elfique",
            "ruler": "Dame Galadriel",
            "capital": "Caras Galadhon",
            "population": "~10,000 Elfes",
            "status": "Domaine protégé",
            "defenses": "Magie elfique, Gardiens",
            "note": "Forêt dorée des Galadhrim"
        },
        "mordor": {
            "name": "MORDOR",
            "type": "Terre des Ténèbres",
            "ruler": "Sauron (détruit)",
            "capital": "Barad-dûr (détruite)",
            "population": "Orcs, Trolls (dispersés)",
            "status": "Royaume détruit",
            "defenses": "Montagnes de l'Ombre",
            "note": "Ancienne forteresse du mal"
        }
    }
    return realms.get(realm, None)

def get_system_status():
    """État des systèmes version fantasy"""
    try:
        import subprocess
        uptime_result = subprocess.run(['uptime'], capture_output=True, text=True, timeout=2)
        load = "1.5" if uptime_result.returncode != 0 else uptime_result.stdout.split()[-3].rstrip(',')
        
        memory_result = subprocess.run(['free', '-h'], capture_output=True, text=True, timeout=2)
        if memory_result.returncode == 0:
            memory_line = [line for line in memory_result.stdout.split('\n') if 'Mem:' in line][0]
            memory = memory_line.split()[2] + "/" + memory_line.split()[1]
        else:
            memory = "3.2G/8G"
            
        disk_result = subprocess.run(['df', '-h', '/'], capture_output=True, text=True, timeout=2)
        disk = "42%" if disk_result.returncode != 0 else disk_result.stdout.split('\n')[1].split()[4]
        
    except:
        load, memory, disk = "1.5", "3.2G/8G", "42%"
    
    return {
        "mana": f"Mana: {load}/10.0",
        "memory": f"Mémoire des Anciens: {memory}",
        "storage": f"Archives de Númenor: {disk}",
        "palantir": "Palantír: ACTIF"
    }

def parse_post_data():
    """Parse les données POST"""
    if os.environ.get("REQUEST_METHOD") != "POST":
        return {}
    
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        if content_length == 0:
            return {}
        
        post_data = sys.stdin.read(content_length)
        return urllib.parse.parse_qs(post_data)
    except:
        return {}

def create_simple_scroll(username, message):
    """Crée un parchemin simple sans problèmes de syntaxe"""
    quote = get_elvish_quote()
    timestamp = datetime.datetime.now().strftime('%d %B %Y - %H:%M:%S')
    
    html = """<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Parchemin Royal</title>
    <style>
        body {
            background-color: #2c1810;
            color: #3c2414;
            font-family: serif;
            height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            margin: 0;
        }
        .scroll {
            background-color: #f4e4bc;
            width: 600px;
            max-width: 90vw;
            padding: 40px;
            border-radius: 15px;
            border: 4px solid #8b7355;
            text-align: center;
            box-shadow: 0 0 30px rgba(0,0,0,0.7);
        }
        .title {
            font-size: 2.5em;
            color: #8b4513;
            margin-bottom: 20px;
            font-weight: bold;
        }
        .subtitle {
            font-size: 1.3em;
            color: #5d4e37;
            margin-bottom: 30px;
            font-style: italic;
        }
        .content {
            font-size: 1.2em;
            line-height: 1.6;
            margin: 25px 0;
            padding: 20px;
            border-left: 3px solid #d4af37;
            border-right: 3px solid #d4af37;
        }
        .quote {
            font-size: 1em;
            color: #8b4513;
            margin: 25px 0;
            padding: 20px;
            background-color: rgba(212, 175, 55, 0.15);
            border-radius: 8px;
            font-style: italic;
        }
        .timestamp {
            font-size: 0.9em;
            color: #8b7355;
            margin-top: 30px;
        }
        .btn {
            position: fixed;
            bottom: 30px;
            right: 30px;
            background-color: #8b4513;
            color: #f4e4bc;
            padding: 15px 25px;
            border-radius: 25px;
            text-decoration: none;
            font-weight: bold;
            border: 2px solid #d4af37;
        }
        .btn:hover {
            background-color: #d4af37;
            color: #2c1810;
        }
    </style>
</head>
<body>
    <div class="scroll">
        <h1 class="title">📜 PROCLAMATION ROYALE 📜</h1>
        <div class="subtitle">~ Archives de Rivendell ~</div>
        <div class="subtitle">Par la présente, il est annoncé :</div>
        
        <div class="content">
            <strong>""" + '"' + escape(message) + '"' + """</strong><br><br>
            <em>— Scellé par """ + escape(username) + """</em>
        </div>
        
        <div class="quote">
            """ + escape(quote) + """
        </div>
        
        <div class="timestamp">
            Proclamé en l'an 2025 du Quatrième Âge de la Terre du Milieu
        </div>
    </div>
    
    <a href="javascript:history.back()" class="btn">← Retour aux Archives</a>
    
    <script>
        setTimeout(function() {
            history.back();
        }, 12000);
    </script>
</body>
</html>"""
    
    return html

def generate_main_interface():
    """Interface principale des Archives de Rivendell"""
    
    query_string = os.environ.get("QUERY_STRING", "")
    params = urllib.parse.parse_qs(query_string)
    
    action = params.get('action', [''])[0]
    character = params.get('character', [''])[0]
    realm = params.get('realm', [''])[0]
    
    response_content = ""
    
    if action == "time":
        now = datetime.datetime.now()
        response_content = f"""
        <div class="crystal-orb">
            <h3>🔮 Orbe de Temps Elfique 🔮</h3>
            <div class="time-display">{now.strftime('%H:%M:%S')}</div>
            <div class="date-info">
                <div class="time-item">
                    <span class="label">Date de la Terre du Milieu:</span>
                    <span class="value">{now.strftime('%d %B %Y')}</span>
                </div>
                <div class="time-item">
                    <span class="label">Jour de l'Année:</span>
                    <span class="value">{now.strftime('%j')}/365</span>
                </div>
                <div class="time-item">
                    <span class="label">Phase Lunaire:</span>
                    <span class="value">{(now.day % 4) + 1}/4</span>
                </div>
                <div class="time-item">
                    <span class="label">Saison:</span>
                    <span class="value">{'Printemps' if now.month in [3,4,5] else 'Été' if now.month in [6,7,8] else 'Automne' if now.month in [9,10,11] else 'Hiver'}</span>
                </div>
            </div>
            <div class="magic-shimmer"></div>
        </div>
        """
    
    elif action == "system":
        status = get_system_status()
        response_content = f"""
        <div class="palantir-status">
            <h3>🌟 État des Palantíri 🌟</h3>
            <div class="palantir-orb">
                <div class="seeing-stone"></div>
                <div class="status-grid">
                    <div class="status-panel">
                        <div class="panel-header">PUISSANCE MAGIQUE</div>
                        <div class="panel-content">{status['mana']}</div>
                        <div class="mana-bar"><div class="mana-fill" style="width: 75%"></div></div>
                    </div>
                    <div class="status-panel">
                        <div class="panel-header">MÉMOIRE DES ANCIENS</div>
                        <div class="panel-content">{status['memory']}</div>
                        <div class="mana-bar"><div class="mana-fill" style="width: 60%"></div></div>
                    </div>
                    <div class="status-panel">
                        <div class="panel-header">ARCHIVES NÚMENOR</div>
                        <div class="panel-content">{status['storage']}</div>
                        <div class="mana-bar"><div class="mana-fill" style="width: 80%"></div></div>
                    </div>
                    <div class="status-panel">
                        <div class="panel-header">CONNEXION MYSTIQUE</div>
                        <div class="panel-content">{status['palantir']}</div>
                        <div class="mana-bar"><div class="mana-fill" style="width: 95%"></div></div>
                    </div>
                </div>
                <div class="status-message">
                    <span class="active">✨ TOUS LES PALANTÍRI SONT ACTIFS ✨</span>
                </div>
            </div>
        </div>
        """
        
    elif action == "character" and character:
        char_info = get_character_info(character)
        parts = char_info.split('|')
        name, race, origin, weapon, achievement, note = parts
        icon, title = get_character_icon(character)
        
        response_content = f"""
        <div class="character-tome">
            <div class="tome-header">
                <h3>📜 Livre des Lignées - Archive Royale 📜</h3>
                <div class="classification">ACCÈS: CONSEIL D'ELROND</div>
            </div>
            <div class="character-portrait">
                <div class="portrait-frame">
                    <div class="elvish-portrait">
                        <div class="character-icon">{icon}</div>
                        <div class="character-title">{escape(title)}</div>
                    </div>
                </div>
                <div class="character-data">
                    <h2 class="character-name">{escape(name)}</h2>
                    <div class="data-row">
                        <span class="data-label">Race:</span>
                        <span class="data-value">{escape(race)}</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Origine:</span>
                        <span class="data-value">{escape(origin)}</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Arme:</span>
                        <span class="data-value">{escape(weapon)}</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Haut Fait:</span>
                        <span class="data-value">{escape(achievement)}</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Notes:</span>
                        <span class="data-value">{escape(note)}</span>
                    </div>
                </div>
            </div>
            <div class="destiny-reading">
                <span class="destiny-label">Lecture du Destin:</span>
                <span class="destiny-value">Lumière Éternelle</span>
            </div>
        </div>
        """
    
    elif action == "realm" and realm:
        realm_data = get_realm_info(realm)
        if realm_data:
            realm_icon = get_realm_icon(realm)
            response_content = f"""
            <div class="realm-scroll">
                <h3>🏰 Archives du Royaume: {realm_data['name']} 🏰</h3>
                <div class="realm-display">
                    <div class="realm-map">
                        <div class="map-icon">{realm_icon}</div>
                        <div class="kingdom-emblem">{realm_data['name']}</div>
                        <div class="map-details"></div>
                    </div>
                    <div class="realm-data">
                        <div class="realm-stat">
                            <span class="stat-label">Type:</span>
                            <span class="stat-value">{escape(realm_data['type'])}</span>
                        </div>
                        <div class="realm-stat">
                            <span class="stat-label">Dirigeant:</span>
                            <span class="stat-value">{escape(realm_data['ruler'])}</span>
                        </div>
                        <div class="realm-stat">
                            <span class="stat-label">Capitale:</span>
                            <span class="stat-value">{escape(realm_data['capital'])}</span>
                        </div>
                        <div class="realm-stat">
                            <span class="stat-label">Population:</span>
                            <span class="stat-value">{escape(realm_data['population'])}</span>
                        </div>
                        <div class="realm-stat">
                            <span class="stat-label">Statut:</span>
                            <span class="stat-value {'good' if 'détruit' not in realm_data['status'] else 'evil'}">{escape(realm_data['status'])}</span>
                        </div>
                        <div class="realm-stat">
                            <span class="stat-label">Défenses:</span>
                            <span class="stat-value">{escape(realm_data['defenses'])}</span>
                        </div>
                    </div>
                </div>
                <div class="realm-note">
                    Note des Scribes: {escape(realm_data['note'])}
                </div>
            </div>
            """
        else:
            response_content = """
            <div class="unknown-realm">
                <h3>🗺️ Terre Inconnue 🗺️</h3>
                <p>Ces terres ne figurent pas dans nos cartes...</p>
                <p>Royaumes connus: gondor, rohan, lothlorien, mordor</p>
            </div>
            """
    
    else:
        quote = get_elvish_quote()
        response_content = f"""
        <div class="welcome-hall">
            <div class="rivendell-gate">
                <h3>🌿 Bienvenue à Rivendell 🌿</h3>
                <div class="gate-sequence">
                    <p class="gate-line">Les portes de la Maison d'Elrond s'ouvrent...</p>
                    <p class="gate-line">Les Archives Elfiques vous accueillent...</p>
                    <p class="gate-line">La sagesse des Anciens est à votre disposition...</p>
                    <p class="gate-line">Que la lumière des Valar vous guide...</p>
                </div>
                <div class="quote-display">
                    <p class="elvish-quote">"{escape(quote)}"</p>
                </div>
            </div>
        </div>
        """
    
    return response_content

def main():
    """Fonction principale"""
    print("Content-Type: text/html; charset=utf-8")
    print("")
    
    if os.environ.get("REQUEST_METHOD") == "POST":
        post_data = parse_post_data()
        username = post_data.get('username', [''])[0]
        message = post_data.get('message', [''])[0]
        
        if username and message:
            print(create_simple_scroll(username, message))
            return
    
    content = generate_main_interface()
    
    print(f"""
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="UTF-8">
        <title>Archives de Rivendell - Terre du Milieu</title>
        <style>
            @import url('https://fonts.googleapis.com/css2?family=Cinzel:wght@300;400;600;700&family=MedievalSharp&display=swap');
            
            * {{
                margin: 0;
                padding: 0;
                box-sizing: border-box;
            }}
            
            body {{
                background: linear-gradient(135deg, #1a2a1a 0%, #2c3e2c 30%, #3d4f3d 70%, #4a5f4a 100%);
                color: #c9b037;
                font-family: 'Cinzel', serif;
                min-height: 100vh;
                position: relative;
                overflow-x: hidden;
            }}
            
            body::before {{
                content: '';
                position: fixed;
                top: 0;
                left: 0;
                width: 100%;
                height: 100%;
                background-image: 
                    radial-gradient(1px 1px at 30px 40px, rgba(201, 176, 55, 0.2), transparent),
                    radial-gradient(1px 1px at 80px 20px, rgba(139, 115, 85, 0.15), transparent),
                    radial-gradient(1px 1px at 150px 90px, rgba(244, 228, 188, 0.1), transparent);
                background-size: 400px 300px;
                animation: subtleParticles 30s linear infinite;
                z-index: -1;
            }}
            
            @keyframes subtleParticles {{
                from {{ transform: translateX(0) translateY(0); }}
                to {{ transform: translateX(-400px) translateY(-300px); }}
            }}
            
            .main-container {{
                max-width: 1200px;
                margin: 0 auto;
                background: linear-gradient(145deg, rgba(44, 35, 25, 0.95), rgba(58, 47, 35, 0.9));
                border: 2px solid #8b7355;
                border-radius: 12px;
                padding: 40px;
                box-shadow: 
                    0 8px 32px rgba(0,0,0,0.4),
                    inset 0 1px 2px rgba(201, 176, 55, 0.1);
                position: relative;
				margin: 0 auto;
            }}
            
            .main-title {{
                text-align: center;
                font-family: 'MedievalSharp', serif;
                font-size: 2.8em;
                font-weight: 700;
                color: #c9b037;
                text-shadow: 
                    2px 2px 4px rgba(0,0,0,0.7),
                    0 0 8px rgba(201, 176, 55, 0.3);
                margin-bottom: 20px;
                letter-spacing: 2px;
            }}
            
            .subtitle {{
                text-align: center;
                font-size: 1.3em;
                color: #9d8f5f;
                margin-bottom: 40px;
                text-shadow: 1px 1px 2px rgba(0,0,0,0.5);
                font-weight: 300;
                font-style: italic;
            }}
            
            .crystal-orb, .palantir-status, .character-tome, .realm-scroll, .welcome-hall, .unknown-realm {{
                background: linear-gradient(145deg, rgba(212, 175, 55, 0.15), rgba(139, 115, 85, 0.1));
                border: 2px solid #d4af37;
                border-radius: 20px;
                padding: 30px;
                margin: 30px 0;
                box-shadow: 
                    0 0 30px rgba(212, 175, 55, 0.3),
                    inset 0 0 20px rgba(212, 175, 55, 0.1);
                position: relative;
            }}
            
            .time-display {{
                font-size: 4em;
                text-align: center;
                color: #f4e4bc;
                text-shadow: 0 0 20px #d4af37, 0 0 40px #d4af37;
                margin: 30px 0;
                font-family: 'MedievalSharp', serif;
            }}
            
            .date-info {{
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
                gap: 20px;
                margin: 30px 0;
            }}
            
            .time-item {{
                background: rgba(26, 58, 26, 0.7);
                padding: 20px;
                border-radius: 15px;
                border: 1px solid #8fbc8f;
                display: flex;
                flex-direction: column;
                align-items: center;
            }}
            
            .label {{
                color: #8fbc8f;
                font-weight: 600;
                margin-bottom: 8px;
            }}
            
            .value {{
                color: #f4e4bc;
                font-size: 1.1em;
            }}
            
            .magic-shimmer {{
                height: 1px;
                background: rgba(212, 175, 55, 0.3);
                margin-top: 20px;
                border-radius: 1px;
                opacity: 0.5;
            }}
            
            .palantir-orb {{
                position: relative;
                text-align: center;
            }}
            
            .seeing-stone {{
                width: 120px;
                height: 120px;
                background: radial-gradient(circle, #d4af37, #8b7355, #2c1810);
                border-radius: 50%;
                margin: 20px auto;
                animation: seeingStone 3s ease-in-out infinite;
                position: relative;
                box-shadow: 0 0 30px #d4af37;
            }}
            
            .seeing-stone::before {{
                content: '';
                position: absolute;
                top: -15px;
                left: -15px;
                right: -15px;
                bottom: -15px;
                border: 2px solid #d4af37;
                border-radius: 50%;
                animation: stoneRing 4s linear infinite;
                opacity: 0.7;
            }}
            
            @keyframes seeingStone {{
                0%, 100% {{ 
                    transform: scale(1);
                    box-shadow: 0 0 30px #d4af37;
                }}
                50% {{ 
                    transform: scale(1.05);
                    box-shadow: 0 0 50px #d4af37, 0 0 80px #d4af37;
                }}
            }}
            
            @keyframes stoneRing {{
                0% {{ transform: rotate(0deg); }}
                100% {{ transform: rotate(360deg); }}
            }}
            
            .status-grid {{
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
                gap: 20px;
                margin: 30px 0;
            }}
            
            .status-panel {{
                background: linear-gradient(145deg, rgba(44, 24, 16, 0.8), rgba(139, 115, 85, 0.6));
                border: 2px solid #8fbc8f;
                border-radius: 15px;
                padding: 20px;
                text-align: center;
            }}
            
            .panel-header {{
                font-size: 0.9em;
                color: #8fbc8f;
                margin-bottom: 15px;
                font-weight: 600;
            }}
            
            .panel-content {{
                font-size: 1.1em;
                color: #f4e4bc;
                margin: 15px 0;
            }}
            
            .mana-bar {{
                width: 100%;
                height: 10px;
                background: rgba(44, 24, 16, 0.8);
                border-radius: 5px;
                overflow: hidden;
                margin-top: 15px;
            }}
            
            .mana-fill {{
                height: 100%;
                background: linear-gradient(90deg, #8fbc8f, #d4af37, #f4e4bc);
                transition: width 2s ease;
                animation: manaFlow 4s linear infinite;
            }}
            
            @keyframes manaFlow {{
                0% {{ filter: brightness(1); }}
                50% {{ filter: brightness(1.3) hue-rotate(15deg); }}
                100% {{ filter: brightness(1); }}
            }}
            
            .active {{
                color: #8fbc8f;
                font-weight: bold;
                font-size: 1.2em;
                text-shadow: 0 0 15px #8fbc8f;
            }}
            
            .tome-header {{
                text-align: center;
                margin-bottom: 30px;
            }}
            
            .classification {{
                background: linear-gradient(45deg, #8b4513, #d4af37);
                color: #2c1810;
                padding: 10px 25px;
                border-radius: 25px;
                display: inline-block;
                font-weight: bold;
                margin-top: 15px;
                animation: classificationPulse 3s infinite;
            }}
            
            @keyframes classificationPulse {{
                0%, 100% {{ opacity: 1; transform: scale(1); }}
                50% {{ opacity: 0.9; transform: scale(1.02); }}
            }}
            
            .character-portrait {{
                display: grid;
                grid-template-columns: 180px 1fr;
                gap: 40px;
                margin: 30px 0;
            }}
            
            .elvish-portrait {{
                width: 180px;
                height: 180px;
                background: linear-gradient(135deg, #2c1810 0%, #3d2a1f 25%, #5d4e37 50%, #8b7355 75%, #a68b5b 100%);
                border: 3px solid #8b7355;
                border-radius: 8px;
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: center;
                color: #d4af37;
                font-weight: bold;
                font-size: 0.85em;
                text-align: center;
                font-family: 'MedievalSharp', serif;
                position: relative;
                box-shadow: 
                    inset 0 0 20px rgba(0,0,0,0.5),
                    0 4px 8px rgba(0,0,0,0.3);
                text-shadow: 1px 1px 2px rgba(0,0,0,0.8);
            }}
            
            .elvish-portrait::before {{
                content: '';
                position: absolute;
                top: 10px;
                left: 10px;
                right: 10px;
                bottom: 10px;
                border: 1px solid rgba(212, 175, 55, 0.3);
                border-radius: 4px;
                background: 
                    radial-gradient(circle at 30% 30%, rgba(212, 175, 55, 0.1), transparent 50%),
                    radial-gradient(circle at 70% 70%, rgba(139, 115, 85, 0.1), transparent 50%);
            }}
            
            .elvish-portrait .character-icon {{
                font-size: 2.5em;
                margin-bottom: 8px;
                opacity: 0.8;
                z-index: 2;
                position: relative;
            }}
            
            .elvish-portrait .character-title {{
                font-size: 0.75em;
                opacity: 0.9;
                z-index: 2;
                position: relative;
            }}
            
            .character-name {{
                font-family: 'MedievalSharp', serif;
                font-size: 2.2em;
                color: #f4e4bc;
                margin-bottom: 25px;
                text-shadow: 0 0 20px #d4af37;
            }}
            
            .data-row {{
                display: flex;
                margin: 15px 0;
                padding: 12px 0;
                border-bottom: 1px solid rgba(212, 175, 55, 0.3);
            }}
            
            .data-label {{
                flex: 0 0 200px;
                color: #8fbc8f;
                font-weight: 600;
            }}
            
            .data-value {{
                flex: 1;
                color: #f4e4bc;
            }}
            
            .destiny-reading {{
                text-align: center;
                margin-top: 25px;
                padding: 20px;
                background: rgba(212, 175, 55, 0.2);
                border-radius: 15px;
                border: 1px solid #d4af37;
            }}
            
            .destiny-value {{
                color: #d4af37;
                font-weight: bold;
                text-shadow: 0 0 10px #d4af37;
                font-family: 'MedievalSharp', serif;
            }}
            
            .realm-display {{
                display: grid;
                grid-template-columns: 250px 1fr;
                gap: 40px;
                margin: 25px 0;
            }}
            
            .realm-map {{
                position: relative;
                width: 100%;
                height: 220px;
                background: linear-gradient(135deg, #2c1810 0%, #3d2a1f 30%, #5d4e37 70%, #8b7355 100%);
                border-radius: 8px;
                border: 2px solid #8b7355;
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: center;
                overflow: hidden;
                box-shadow: 
                    inset 0 0 30px rgba(0,0,0,0.6),
                    0 4px 12px rgba(0,0,0,0.4);
            }}
            
            .kingdom-emblem {{
                font-family: 'MedievalSharp', serif;
                font-size: 1.3em;
                color: #d4af37;
                text-shadow: 2px 2px 4px rgba(0,0,0,0.8);
                text-align: center;
                z-index: 3;
                position: relative;
                margin-bottom: 10px;
            }}
            
            .map-icon {{
                font-size: 3em;
                color: rgba(212, 175, 55, 0.7);
                margin-bottom: 5px;
                z-index: 2;
                position: relative;
                text-shadow: 0 0 10px rgba(212, 175, 55, 0.5);
            }}
            
            .map-details {{
                position: absolute;
                top: 0;
                left: 0;
                right: 0;
                bottom: 0;
                background-image: 
                    linear-gradient(45deg, transparent 30%, rgba(212, 175, 55, 0.05) 50%, transparent 70%),
                    radial-gradient(circle at 20% 80%, rgba(139, 115, 85, 0.1), transparent 40%),
                    radial-gradient(circle at 80% 20%, rgba(212, 175, 55, 0.08), transparent 30%);
                z-index: 1;
            }}
            
            .map-details::before {{
                content: '';
                position: absolute;
                top: 15px;
                left: 15px;
                right: 15px;
                bottom: 15px;
                border: 1px solid rgba(212, 175, 55, 0.2);
                border-radius: 4px;
            }}
            
            .realm-stat {{
                display: flex;
                justify-content: space-between;
                padding: 10px 0;
                border-bottom: 1px solid rgba(212, 175, 55, 0.2);
            }}
            
            .stat-label {{
                font-weight: 600;
                color: #8fbc8f;
            }}
            
            .stat-value {{
                color: #f4e4bc;
            }}
            
            .stat-value.good {{
                color: #8fbc8f;
            }}
            
            .stat-value.evil {{
                color: #cd5c5c;
            }}
            
            .realm-note {{
                margin-top: 25px;
                padding: 20px;
                background: rgba(139, 115, 85, 0.1);
                border-radius: 10px;
                border-left: 4px solid #d4af37;
                font-style: italic;
            }}
            
            .rivendell-gate {{
                text-align: center;
                font-family: 'Cinzel', serif;
            }}
            
            .gate-sequence {{
                margin: 30px 0;
                text-align: left;
                max-width: 600px;
                margin-left: auto;
                margin-right: auto;
            }}
            
            .gate-line {{
                margin: 12px 0;
                opacity: 0;
                animation: gateLine 1s ease forwards;
                color: #8fbc8f;
            }}
            
            .gate-line:nth-child(1) {{ animation-delay: 0.5s; }}
            .gate-line:nth-child(2) {{ animation-delay: 1s; }}
            .gate-line:nth-child(3) {{ animation-delay: 1.5s; }}
            .gate-line:nth-child(4) {{ animation-delay: 2s; }}
            
            @keyframes gateLine {{
                to {{ opacity: 1; }}
            }}
            
            .quote-display {{
                margin-top: 40px;
                padding: 25px;
                background: rgba(44, 24, 16, 0.5);
                border-radius: 15px;
                border-left: 4px solid #d4af37;
            }}
            
            .elvish-quote {{
                font-family: 'MedievalSharp', serif;
                font-style: italic;
                color: #d4af37;
                font-size: 1.3em;
                text-shadow: 0 0 10px #d4af37;
            }}
            
            .scroll-section {{
                background: rgba(44, 24, 16, 0.8);
                border: 2px solid #8b7355;
                border-radius: 20px;
                padding: 30px;
                margin: 30px 0;
                position: relative;
                box-shadow: 0 4px 12px rgba(0,0,0,0.3);
            }}
            
            .archive-buttons {{
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
                gap: 20px;
                margin: 30px 0;
            }}
            
            .elvish-btn {{
                background: linear-gradient(145deg, rgba(44, 24, 16, 0.9), rgba(139, 115, 85, 0.7));
                border: 2px solid #8fbc8f;
                color: #8fbc8f;
                padding: 20px 30px;
                text-decoration: none;
                border-radius: 15px;
                font-family: 'Cinzel', serif;
                font-weight: 600;
                text-align: center;
                transition: all 0.4s ease;
                position: relative;
                overflow: hidden;
                text-transform: capitalize;
            }}
            
            .elvish-btn::before {{
                content: '';
                position: absolute;
                top: 0;
                left: -100%;
                width: 100%;
                height: 100%;
                background: linear-gradient(90deg, transparent, rgba(212, 175, 55, 0.3), transparent);
                transition: left 0.6s;
            }}
            
            .elvish-btn:hover::before {{
                left: 100%;
            }}
            
            .elvish-btn:hover {{
                background: linear-gradient(145deg, #d4af37, #f4e4bc);
                color: #2c1810;
                box-shadow: 
                    0 0 30px rgba(212, 175, 55, 0.6),
                    0 0 50px rgba(212, 175, 55, 0.4);
                transform: translateY(-3px) scale(1.02);
                border-color: #d4af37;
            }}
            
            .scroll-form {{
                background: rgba(26, 58, 26, 0.9);
                border: 2px solid #8fbc8f;
                border-radius: 20px;
                padding: 30px;
                margin: 30px 0;
            }}
            
            .form-input {{
                width: 100%;
                max-width: 600px;
                padding: 18px;
                background: rgba(44, 24, 16, 0.8);
                border: 2px solid #8fbc8f;
                color: #f4e4bc;
                border-radius: 10px;
                font-family: 'Cinzel', serif;
                margin: 15px 0;
                font-size: 16px;
                transition: all 0.3s ease;
            }}
            
            .form-input:focus {{
                outline: none;
                border-color: #d4af37;
                box-shadow: 0 0 20px rgba(212, 175, 55, 0.5);
                color: #d4af37;
                transform: scale(1.02);
            }}
            
            .form-input::placeholder {{
                color: #8fbc8f;
                opacity: 0.7;
            }}
            
            .send-btn {{
                background: linear-gradient(145deg, #8b4513, #a0522d);
                border: 3px solid #d4af37;
                color: #f4e4bc;
                padding: 18px 35px;
                font-family: 'Cinzel', serif;
                font-weight: bold;
                border-radius: 15px;
                cursor: pointer;
                transition: all 0.4s ease;
                box-shadow: 0 0 25px rgba(139, 69, 19, 0.4);
                text-transform: uppercase;
                letter-spacing: 1px;
            }}
            
            .send-btn:hover {{
                background: linear-gradient(145deg, #d4af37, #f4e4bc);
                color: #2c1810;
                box-shadow: 
                    0 0 40px rgba(212, 175, 55, 0.7),
                    0 0 60px rgba(212, 175, 55, 0.5);
                transform: scale(1.1) translateY(-2px);
            }}
            
            .diagnostic-scroll {{
                background: rgba(44, 24, 16, 0.95);
                border: 2px solid #8b7355;
                padding: 25px;
                margin: 25px 0;
                border-radius: 15px;
                font-size: 0.95em;
                color: #8b7355;
            }}
            
            .diagnostic-scroll h4 {{
                color: #d4af37;
                margin-bottom: 15px;
                font-size: 1.1em;
                font-family: 'MedievalSharp', serif;
            }}
            
            @media (max-width: 768px) {{
                .main-container {{
                    margin: 10px;
                    padding: 20px;
                }}
                
                .main-title {{
                    font-size: 2.2em;
                }}
                
                .realm-display, .character-portrait {{
                    grid-template-columns: 1fr;
                    text-align: center;
                }}
                
                .archive-buttons {{
                    grid-template-columns: 1fr;
                }}
            }}
        </style>
    </head>
    <body>
        <div class="main-container">
            <h1 class="main-title">ARCHIVES DE RIVENDELL</h1>
            <h2 class="subtitle">Bibliothèque Elfique - Terre du Milieu</h2>
            
            {content}
            
            <div class="scroll-form">
                <h3 style="color: #d4af37; text-align: center; margin-bottom: 25px; font-family: 'MedievalSharp', serif;">📜 Nouveau Parchemin</h3>
                <p style="text-align: center; margin-bottom: 25px; color: #8fbc8f;">Confiez vos écrits aux Archives Elfiques</p>
                <form method="POST" action="" style="display: flex; flex-direction: column; align-items: center;">
                    <input type="text" name="username" placeholder="Nom du Scribe" required class="form-input">
                    <textarea name="message" rows="5" placeholder="Récit à archiver..." required class="form-input"></textarea>
                    <button type="submit" class="send-btn">🌿 Sceller le Parchemin</button>
                </form>
            </div>
            
            <div class="scroll-section">
                <h3 style="color: #8fbc8f; text-align: center; margin-bottom: 25px; font-family: 'MedievalSharp', serif;">✨ Archives de la Terre du Milieu ✨</h3>
                <p style="text-align: center; margin-bottom: 25px;">Explorez les connaissances des Anciens Jours</p>
                <div class="archive-buttons">
                    <a href="?action=time" class="elvish-btn">🔮 Orbe de Temps Elfique</a>
                    <a href="?action=system" class="elvish-btn">🌟 État des Palantíri</a>
                    <a href="?action=character&character=gandalf" class="elvish-btn">🧙 Gandalf le Gris</a>
                    <a href="?action=character&character=legolas" class="elvish-btn">🏹 Legolas du Royaume Sylvestre</a>
                    <a href="?action=character&character=aragorn" class="elvish-btn">⚔️ Aragorn, Roi du Gondor</a>
                    <a href="?action=character&character=galadriel" class="elvish-btn">✨ Dame Galadriel</a>
                    <a href="?action=realm&realm=gondor" class="elvish-btn">🏰 Royaume du Gondor</a>
                    <a href="?action=realm&realm=lothlorien" class="elvish-btn">🌳 Lothlórien</a>
                </div>
            </div>
            
            <div class="diagnostic-scroll">
                <h4>🔍 Diagnostic des Palantíri</h4>
                <p><strong>Méthode de Communication:</strong> {os.environ.get("REQUEST_METHOD", "Inconnue")}</p>
                <p><strong>Paramètres Mystiques:</strong> {os.environ.get("QUERY_STRING", "Aucun")}</p>
                <p><strong>Heure de Rivendell:</strong> {datetime.datetime.now().strftime('%d %B %Y - %H:%M:%S')}</p>
                <p><strong>Port des Archives:</strong> {os.environ.get("SERVER_PORT", "Mystique")}</p>
                <p><strong>Terminal Elfique:</strong> {os.environ.get("SCRIPT_NAME", "Archives de Rivendell")}</p>
                <p><strong>Statut:</strong> <span style="color: #8fbc8f;">✨ TOUS LES PALANTÍRI BRILLENT ✨</span></p>
            </div>
        </div>
        
        <script>
            function createMagicParticles() {{
                const numberOfParticles = 50;
                for (let i = 0; i < numberOfParticles; i++) {{
                    const particle = document.createElement('div');
                    particle.style.position = 'fixed';
                    particle.style.left = Math.random() * 100 + '%';
                    particle.style.top = Math.random() * 100 + '%';
                    particle.style.width = Math.random() * 3 + 1 + 'px';
                    particle.style.height = particle.style.width;
                    particle.style.background = Math.random() > 0.5 ? '#d4af37' : '#8fbc8f';
                    particle.style.borderRadius = '50%';
                    particle.style.opacity = Math.random() * 0.7 + 0.3;
                    particle.style.animation = 'magicFloat ' + (Math.random() * 4 + 3) + 's infinite ease-in-out';
                    particle.style.zIndex = '-1';
                    document.body.appendChild(particle);
                }}
            }}
            
            const style = document.createElement('style');
            style.textContent = '@keyframes magicFloat {{ 0%, 100% {{ transform: translateY(0px) rotate(0deg); opacity: 0.3; }} 50% {{ transform: translateY(-20px) rotate(180deg); opacity: 0.8; }} }}';
            document.head.appendChild(style);
            
            createMagicParticles();
        </script>
    </body>
    </html>
    """)

if __name__ == "__main__":
    main()
