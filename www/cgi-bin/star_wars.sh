#!/bin/bash

# Function to properly decode URL encoded strings
urldecode() {
    local url_encoded="${1//+/ }"
    printf '%b' "${url_encoded//%/\\x}"
}

# Function to generate random Star Wars quotes
get_random_quote() {
    local quotes=(
        "Que la Force soit avec toi"
        "Je suis ton père"
        "Il y a toujours un plus gros poisson"
        "Fais ou ne fais pas, il n'y a pas d'essai"
        "La peur mène à la colère, la colère mène à la haine"
        "Ces droïdes ne sont pas ceux que vous cherchez"
        "Jamais sous-estimez le pouvoir du côté obscur"
        "Ta destinée t'attend, jeune Skywalker"
        "Je trouve votre manque de foi déconcertant"
        "C'est un piège !"
    )
    local random_index=$((RANDOM % ${#quotes[@]}))
    echo "${quotes[$random_index]}"
}

# Function to get character info with more details
get_character_info() {
    local character="$1"
    case "$character" in
        "luke")
            echo "Luke Skywalker|Chevalier Jedi|Tatooine → Dagobah|Sabre laser vert|A chopé sa soeur|Fils d'Anakin Skywalker"
            ;;
        "vader")
            echo "Dark Vador|Seigneur Sith|Mustafar → Death Star|Sabre laser rouge|A cuit comme une merguez|Ancien Anakin Skywalker"
            ;;
        "leia")
            echo "Princesse Leia Organa|Leader Rebelle|Alderaan|Blaster + Diplomatie|A chopé son frère|Sœur jumelle de Luke"
            ;;
        "han")
            echo "Han Solo|Contrebandier|Corellia|Blaster DL-44|A tiré en premier|Ami fidèle et pilote hors pair"
            ;;
        "yoda")
            echo "Maître Yoda|Grand Maître Jedi|Dagobah|Sabre laser + Force|900 ans d'âge|Entraîneur de Luke Skywalker"
            ;;
        "obiwan")
            echo "Obi-Wan Kenobi|Maître Jedi|Tatooine|Sabre laser bleu|Ermite de Ben Kenobi|Maître d'Anakin et Luke"
            ;;
        "jon")
            echo "Jonny la chipie|Gros beauf|Waterloo|Sa calvasse réfléchit le soleil|A fait Cub3D et minishell solo|Sait pas réparer son vélo"
            ;;
        "ed")
            echo "Edoulazone|Gros beauf|BX|Ses pets font fondre le métal|Va finir le common core seulement avec des fonctions de parsing|Est sacrément con"
            ;;
		"sim")
			echo "Rock angel|Gros beauf|Tournai|Sa moustache fait fondre les coeurs|A mis plus de temps sur Pipex que sur Minishell|A le pire sens de l'orientation de toute la galaxie"
			;;
        *)
            echo "Agent Inconnu|Classification: INCONNUE|Outer Rim|Arme: CLASSIFIÉE|Mission: REDACTED|Statut: EN SURVEILLANCE"
            ;;
    esac
}

# Function to get enhanced system status
get_system_status() {
    local cpu_load=$(uptime | grep -o '[0-9]\+\.[0-9]\+' | head -1)
    local memory_info=$(free -h 2>/dev/null | grep Mem | awk '{print $3"/"$2}' || echo "Unknown")
    local disk_usage=$(df -h / 2>/dev/null | tail -1 | awk '{print $5}' || echo "Unknown")
    local uptime_info=$(uptime -p 2>/dev/null || echo "Unknown")
    
    echo "CPU: ${cpu_load:-Unknown}%|Memory: $memory_info|Disk: ${disk_usage:-Unknown}|Uptime: $uptime_info"
}

# Main execution
echo "Content-Type: text/html; charset=utf-8"
echo ""

# *** CORRECTION DU PARSING POST ***
# Parse POST data if present
if [ "$REQUEST_METHOD" = "POST" ] && [ -n "$CONTENT_LENGTH" ]; then
    POST_DATA=$(head -c "$CONTENT_LENGTH")
    
    # Extract username and message from POST data
    if [ -n "$POST_DATA" ]; then
        username=$(echo "$POST_DATA" | grep -o 'username=[^&]*' | cut -d'=' -f2- | head -1)
        message=$(echo "$POST_DATA" | grep -o 'message=[^&]*' | cut -d'=' -f2- | head -1)
        
        # URL decode
        username=$(urldecode "$username")
        message=$(urldecode "$message")
        
        # Generate crawl response for POST
        if [ -n "$username" ] && [ -n "$message" ]; then
            cat << 'CRAWL_HTML'
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Transmission Reçue - Empire Galactique</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        
        body {
            background: #000;
            color: #ffff00;
            font-family: 'Courier New', monospace;
            overflow: hidden;
            height: 100vh;
        }
        
        .starfield {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-image: 
                radial-gradient(2px 2px at 20px 30px, rgba(255,255,255,0.8), transparent),
                radial-gradient(2px 2px at 40px 70px, rgba(255,255,255,0.6), transparent),
                radial-gradient(1px 1px at 90px 40px, rgba(255,255,255,0.9), transparent),
                radial-gradient(1px 1px at 130px 80px, rgba(255,255,255,0.7), transparent),
                radial-gradient(2px 2px at 160px 30px, rgba(255,255,255,0.5), transparent);
            background-size: 200px 150px;
            animation: starfield 30s linear infinite;
            z-index: -1;
        }
        
        @keyframes starfield {
            from { transform: translateX(0) translateY(0); }
            to { transform: translateX(-200px) translateY(-150px); }
        }
        
        .crawl-container {
            position: relative;
            width: 100%;
            height: 100%;
            perspective: 400px;
            overflow: hidden;
        }
        
        .crawl {
            position: relative;
            top: 100%;
            transform-origin: 50% 100%;
            animation: crawl 50s linear forwards;
            color: #ffff00;
            font-size: 18px;
            font-weight: bold;
            text-align: center;
            line-height: 1.8;
            letter-spacing: 2px;
            max-width: 600px;
            margin: 0 auto;
        }
        
        .crawl .title h1 {
            font-size: 3em;
            margin-bottom: 20px;
        }
        
        .crawl .title h2 {
            font-size: 1.8em;
            margin-bottom: 30px;
        }
        
        .crawl .episode {
            font-size: 1.2em;
            margin-bottom: 40px;
            opacity: 0.8;
        }
        
        .crawl .content {
            font-size: 1.1em;
        }
        
        @keyframes crawl {
            0% {
                top: 100%;
                transform: rotateX(20deg) translateZ(0);
            }
            100% {
                top: -150%;
                transform: rotateX(25deg) translateZ(-2000px);
            }
        }
        
        .return-btn {
            position: fixed;
            bottom: 30px;
            right: 30px;
            background: linear-gradient(145deg, #ff0000, #cc0000);
            border: 2px solid #ff0000;
            color: #fff;
            padding: 15px 25px;
            border-radius: 8px;
            text-decoration: none;
            font-family: 'Courier New', monospace;
            font-weight: bold;
            z-index: 100;
            transition: all 0.3s ease;
        }
        
        .return-btn:hover {
            background: linear-gradient(145deg, #ffff00, #ff6600);
            color: #000;
            box-shadow: 0 0 20px rgba(255,255,0,0.6);
        }
    </style>
</head>
<body>
    <div class="starfield"></div>
    <div class="crawl-container">
        <div class="crawl">
            <div class="title">
                <h1>RAPPORT DE MISSION</h1>
                <h2>Agent: 
CRAWL_HTML
            echo "$username</h2>"
            cat << 'CRAWL_HTML2'
            </div>
            <div class="episode">Episode IV</div>
            <div class="content">
                <p>Une transmission a été reçue</p>
                <p>de l'agent 
CRAWL_HTML2
            echo "$username, en mission"
            cat << 'CRAWL_HTML3'
                </p>
                <p>dans les territoires de l'Outer Rim.</p>
                <br>
                <p>Le rapport indique:</p>
                <p>"
CRAWL_HTML3
            echo "$message"
            cat << 'CRAWL_HTML4'
                "</p>
                <br>
                <p>Cette information cruciale</p>
                <p>permettra à l'Alliance Rebelle</p>
                <p>de poursuivre sa lutte</p>
                <p>contre l'Empire Galactique.</p>
                <br>
                <p>La Force guide nos pas</p>
                <p>vers la liberté et la justice.</p>
                <p>
CRAWL_HTML4
            echo "$(get_random_quote)"
            cat << 'CRAWL_HTML5'
                </p>
                <br>
                <p>Fin de transmission...</p>
                <p>
CRAWL_HTML5
            echo "$(date '+%d/%m/%Y - %H:%M:%S')"
            cat << 'CRAWL_HTML6'
                </p>
            </div>
        </div>
    </div>
    
    <a href="javascript:history.back()" class="return-btn">← Retour au Terminal</a>
    
    <script>
        // Auto-return after crawl
        setTimeout(() => {
            history.back();
        }, 50000);
    </script>
</body>
</html>
CRAWL_HTML6
            exit 0
        else
            # POST error
            RESPONSE="<div class='sith-error'>
                <h3>🔴 ERREUR DE TRANSMISSION 🔴</h3>
                <p>Données de mission corrompues par le côté obscur...</p>
                <div class='static-effect'></div>
            </div>"
        fi
    fi
fi

# *** GESTION DES REQUÊTES GET ***
# Parse query parameters for GET requests
action=""
character=""
planet=""

if [ -n "$QUERY_STRING" ]; then
    if [[ "$QUERY_STRING" =~ action=([^&]*) ]]; then
        action=$(urldecode "${BASH_REMATCH[1]}")
    fi
    if [[ "$QUERY_STRING" =~ character=([^&]*) ]]; then
        character=$(urldecode "${BASH_REMATCH[1]}")
    fi
    if [[ "$QUERY_STRING" =~ planet=([^&]*) ]]; then
        planet=$(urldecode "${BASH_REMATCH[1]}")
    fi
fi

# Generate GET responses
case "$action" in
    "time")
        RESPONSE="<div class='hologram-display time-module'>
            <div class='hologram-flicker'>
                <h3>⏰ CHRONOMETRES GALACTIQUES ⏰</h3>
                <div class='time-display imperial-time'>$(date '+%H:%M:%S')</div>
                <div class='date-grid'>
                    <div class='date-item'>
                        <span class='label'>Date Terrienne:</span>
                        <span class='value'>$(date '+%d/%m/%Y')</span>
                    </div>
                    <div class='date-item'>
                        <span class='label'>Jour Standard:</span>
                        <span class='value'>$(date '+%j')/365</span>
                    </div>
                    <div class='date-item'>
                        <span class='label'>Cycle Galactique:</span>
                        <span class='value'>$(date '+%W')</span>
                    </div>
                    <div class='date-item'>
                        <span class='label'>Phase Lunaire:</span>
                        <span class='value'>$(( $(date +%d) % 4 + 1 ))/4</span>
                    </div>
                </div>
                <div class='scanner-sweep'></div>
            </div>
        </div>"
        ;;
    "system")
        status=$(get_system_status)
        cpu=$(echo "$status" | cut -d'|' -f1)
        memory=$(echo "$status" | cut -d'|' -f2)
        disk=$(echo "$status" | cut -d'|' -f3)
        uptime=$(echo "$status" | cut -d'|' -f4)
        
        RESPONSE="<div class='death-star-status'>
            <h3>💀 ÉTAT DE L'ÉTOILE DE LA MORT 💀</h3>
            <div class='reactor-core'>
                <div class='core-animation'></div>
                <div class='system-grid'>
                    <div class='system-panel'>
                        <div class='panel-header'>RÉACTEURS PRINCIPAUX</div>
                        <div class='panel-content'>$cpu</div>
                        <div class='power-bar'><div class='power-fill' style='width: 85%'></div></div>
                    </div>
                    <div class='system-panel'>
                        <div class='panel-header'>MÉMOIRE HOLOGRAPHIQUE</div>
                        <div class='panel-content'>$memory</div>
                        <div class='power-bar'><div class='power-fill' style='width: 70%'></div></div>
                    </div>
                    <div class='system-panel'>
                        <div class='panel-header'>STOCKAGE CRISTALLIN</div>
                        <div class='panel-content'>$disk utilisé</div>
                        <div class='power-bar'><div class='power-fill' style='width: 60%'></div></div>
                    </div>
                    <div class='system-panel'>
                        <div class='panel-header'>TEMPS OPÉRATIONNEL</div>
                        <div class='panel-content'>$uptime</div>
                        <div class='power-bar'><div class='power-fill' style='width: 95%'></div></div>
                    </div>
                </div>
                <div class='status-message'>
                    <span class='operational'>🟢 TOUS SYSTÈMES OPÉRATIONNELS</span>
                </div>
            </div>
        </div>"
        ;;
    "character")
        if [ -n "$character" ]; then
            info=$(get_character_info "$character")
            name=$(echo "$info" | cut -d'|' -f1)
            role=$(echo "$info" | cut -d'|' -f2)
            origin=$(echo "$info" | cut -d'|' -f3)
            weapon=$(echo "$info" | cut -d'|' -f4)
            achievement=$(echo "$info" | cut -d'|' -f5)
            note=$(echo "$info" | cut -d'|' -f6)
            
            RESPONSE="<div class='character-dossier'>
                <div class='dossier-header'>
                    <h3>📋 DOSSIER PERSONNEL CLASSIFIÉ 📋</h3>
                    <div class='classification'>NIVEAU DE SÉCURITÉ: ALPHA</div>
                </div>
                <div class='character-profile'>
                    <div class='profile-image'>
                        <div class='hologram-portrait'>$name</div>
                    </div>
                    <div class='profile-data'>
                        <h2 class='character-name'>$name</h2>
                        <div class='data-line'>
                            <span class='data-label'>Classification:</span>
                            <span class='data-value'>$role</span>
                        </div>
                        <div class='data-line'>
                            <span class='data-label'>Origine/Localisation:</span>
                            <span class='data-value'>$origin</span>
                        </div>
                        <div class='data-line'>
                            <span class='data-label'>Armement:</span>
                            <span class='data-value'>$weapon</span>
                        </div>
                        <div class='data-line'>
                            <span class='data-label'>Accomplissement:</span>
                            <span class='data-value'>$achievement</span>
                        </div>
                        <div class='data-line'>
                            <span class='data-label'>Notes:</span>
                            <span class='data-value'>$note</span>
                        </div>
                    </div>
                </div>
                <div class='threat-assessment'>
                    <span class='assessment-label'>ÉVALUATION DE MENACE:</span>
                    <span class='threat-level threat-high'>ÉLEVÉE</span>
                </div>
            </div>"
        else
            RESPONSE="<div class='error-display'>
                <h3>🚫 ACCÈS REFUSÉ 🚫</h3>
                <p>Spécifiez un code d'agent valide:</p>
                <p>luke, vader, leia, han, yoda, obiwan, jon, ed ,sim</p>
            </div>"
        fi
        ;;
    "planet")
        if [ -n "$planet" ]; then
            case "$planet" in
                "tatooine")
                    RESPONSE="<div class='planetary-scan'>
                        <h3>🏜️ ANALYSE PLANÉTAIRE: TATOOINE 🏜️</h3>
                        <div class='scan-interface'>
                            <div class='planet-visual'>
                                <div class='planet-surface tatooine'></div>
                                <div class='orbit-ring'></div>
                            </div>
                            <div class='scan-data'>
                                <div class='scan-result'>
                                    <span class='param'>Type:</span>
                                    <span class='value'>Planète désertique</span>
                                </div>
                                <div class='scan-result'>
                                    <span class='param'>Soleils:</span>
                                    <span class='value'>2 étoiles (système binaire)</span>
                                </div>
                                <div class='scan-result'>
                                    <span class='param'>Population:</span>
                                    <span class='value'>~200,000 habitants</span>
                                </div>
                                <div class='scan-result'>
                                    <span class='param'>Contrôle:</span>
                                    <span class='value warning'>Cartel des Hutts</span>
                                </div>
                                <div class='scan-result'>
                                    <span class='param'>Ressources:</span>
                                    <span class='value'>Métaux rares, épices</span>
                                </div>
                                <div class='scan-result'>
                                    <span class='param'>Menaces:</span>
                                    <span class='value danger'>Raiders Tusken, Krayt Dragons</span>
                                </div>
                            </div>
                        </div>
                        <div class='strategic-note'>
                            Note: Monde natal de Luke et Anakin Skywalker
                        </div>
                    </div>"
                    ;;
                "coruscant")
                    RESPONSE="<div class='planetary-scan'>
                        <h3>🌆 ANALYSE PLANÉTAIRE: CORUSCANT 🌆</h3>
                        <div class='scan-interface'>
                            <div class='planet-visual'>
                                <div class='planet-surface coruscant'></div>
                                <div class='city-lights'></div>
                            </div>
                            <div class='scan-data'>
                                <div class='scan-result'>
                                    <span class='param'>Type:</span>
                                    <span class='value'>Planète-Métropole</span>
                                </div>
                                <div class='scan-result'>
                                    <span class='param'>Population:</span>
                                    <span class='value'>1+ trillion d'habitants</span>
                                </div>
                                <div class='scan-result'>
                                    <span class='param'>Contrôle:</span>
                                    <span class='value imperial'>EMPIRE GALACTIQUE</span>
                                </div>
                                <div class='scan-result'>
                                    <span class='param'>Statut:</span>
                                    <span class='value imperial'>CAPITALE IMPÉRIALE</span>
                                </div>
                                <div class='scan-result'>
                                    <span class='param'>Défenses:</span>
                                    <span class='value'>Boucliers planétaires, Flotte Impériale</span>
                                </div>
                                <div class='scan-result'>
                                    <span class='param'>Niveau de Sécurité:</span>
                                    <span class='value imperial'>MAXIMUM</span>
                                </div>
                            </div>
                        </div>
                        <div class='strategic-note imperial'>
                            🔴 ZONE HAUTEMENT SÉCURISÉE - ACCÈS INTERDIT AUX REBELLES
                        </div>
                    </div>"
                    ;;
                *)
                    RESPONSE="<div class='scanning-display'>
                        <h3>🔍 SCAN PLANÉTAIRE EN COURS 🔍</h3>
                        <div class='scan-progress'>
                            <div class='scan-target'>$(echo $planet | tr '[:lower:]' '[:upper:]')</div>
                            <div class='progress-bars'>
                                <div class='progress-line'>
                                    <span>Analyse atmosphérique:</span>
                                    <div class='bar'><div class='fill' style='width: 75%'></div></div>
                                </div>
                                <div class='progress-line'>
                                    <span>Scan géologique:</span>
                                    <div class='bar'><div class='fill' style='width: 45%'></div></div>
                                </div>
                                <div class='progress-line'>
                                    <span>Détection de vie:</span>
                                    <div class='bar'><div class='fill' style='width: 30%'></div></div>
                                </div>
                            </div>
                            <p class='scan-status'>Données insuffisantes dans nos archives...</p>
                            <p class='scan-suggestion'>Planètes disponibles: tatooine, coruscant</p>
                        </div>
                    </div>"
                    ;;
            esac
        else
            RESPONSE="<div class='error-display'>Spécifiez une planète à analyser</div>"
        fi
        ;;
    *)
        RESPONSE="<div class='welcome-terminal'>
            <div class='terminal-boot'>
                <h3>🌟 TERMINAL IMPÉRIAL ACTIVÉ 🌟</h3>
                <div class='boot-sequence'>
                    <p class='boot-line'>Initialisation des systèmes...</p>
                    <p class='boot-line'>Connexion au réseau HoloNet...</p>
                    <p class='boot-line'>Authentification impériale validée</p>
                    <p class='boot-line'>Accès autorisé aux bases de données</p>
                </div>
                <div class='quote-display'>
                    <p class='quote'>\"$(get_random_quote)\"</p>
                </div>
            </div>
        </div>"
        ;;
esac

# Output the main interface HTML (same CSS as before)
cat << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Terminal Impérial - Réseau HoloNet</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@300;400;700;900&display=swap');
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            background: radial-gradient(ellipse at center, #0a0a23 0%, #000000 70%);
            color: #00ff41;
            font-family: 'Orbitron', 'Courier New', monospace;
            min-height: 100vh;
            overflow-x: hidden;
            position: relative;
        }
        
        /* Starfield animation */
        body::before {
            content: '';
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-image: 
                radial-gradient(2px 2px at 20px 30px, rgba(255,255,255,0.8), transparent),
                radial-gradient(2px 2px at 40px 70px, rgba(255,255,255,0.6), transparent),
                radial-gradient(1px 1px at 90px 40px, rgba(255,255,255,0.9), transparent),
                radial-gradient(1px 1px at 130px 80px, rgba(255,255,255,0.7), transparent),
                radial-gradient(2px 2px at 160px 30px, rgba(255,255,255,0.5), transparent);
            background-size: 200px 150px;
            animation: starfield 30s linear infinite;
            z-index: -1;
        }
        
        @keyframes starfield {
            from { transform: translateX(0) translateY(0); }
            to { transform: translateX(-200px) translateY(-150px); }
        }
        
        .main-container {
            max-width: 1200px;
            margin: 0 auto;
            background: linear-gradient(145deg, rgba(0,0,0,0.95), rgba(20,20,40,0.9));
            border: 3px solid;
            border-image: linear-gradient(45deg, #00ff41, #0080ff, #ff0080, #ffff00) 1;
            border-radius: 20px;
            padding: 30px;
s            box-shadow: 
                0 0 50px rgba(0,255,65,0.3),
                inset 0 0 50px rgba(0,255,65,0.1);
            position: relative;
			margin: 0 auto;
        }
        
        .main-title {
            text-align: center;
            font-size: 2.8em;
            font-weight: 900;
            color: #ff0040;
            text-shadow: 
                0 0 10px #ff0040,
                0 0 20px #ff0040,
                0 0 30px #ff0040;
            margin-bottom: 15px;
            animation: title-pulse 3s ease-in-out infinite alternate;
        }
        
        .subtitle {
            text-align: center;
            font-size: 1.3em;
            color: #ffff00;
            margin-bottom: 40px;
            text-shadow: 0 0 15px #ffff00;
        }
        
        @keyframes title-pulse {
            from { 
                text-shadow: 0 0 10px #ff0040, 0 0 20px #ff0040, 0 0 30px #ff0040;
                transform: scale(1);
            }
            to { 
                text-shadow: 0 0 20px #ff0040, 0 0 30px #ff0040, 0 0 40px #ff0040;
                transform: scale(1.02);
            }
        }
        
        /* Enhanced component styles */
        .hologram-display, .death-star-status, .character-dossier, .planetary-scan, .welcome-terminal, .scanning-display {
            background: linear-gradient(145deg, rgba(0,255,255,0.1), rgba(0,100,255,0.1));
            border: 2px solid #00ffff;
            border-radius: 15px;
            padding: 25px;
            margin: 25px 0;
            box-shadow: 
                0 0 30px rgba(0,255,255,0.3),
                inset 0 0 20px rgba(0,255,255,0.1);
            position: relative;
        }
        
        .death-star-status {
            background: linear-gradient(145deg, rgba(255,0,0,0.2), rgba(100,0,0,0.1));
            border: 3px solid #ff0000;
            box-shadow: 0 0 40px rgba(255,0,0,0.4);
        }
        
        .character-dossier {
            background: linear-gradient(145deg, rgba(0,100,255,0.2), rgba(0,50,150,0.1));
            border: 3px solid #0080ff;
            box-shadow: 0 0 30px rgba(0,128,255,0.4);
        }
        
        .planetary-scan {
            background: linear-gradient(145deg, rgba(100,255,100,0.1), rgba(50,200,50,0.1));
            border: 2px solid #00ff00;
            box-shadow: 0 0 30px rgba(0,255,0,0.3);
        }
        
        .hologram-flicker {
            animation: hologram-effect 2s ease-in-out infinite;
        }
        
        @keyframes hologram-effect {
            0%, 100% { opacity: 1; filter: brightness(1); }
            50% { opacity: 0.9; filter: brightness(1.1); }
        }
        
        .imperial-time {
            font-size: 4em;
            text-align: center;
            color: #ffff00;
            text-shadow: 
                0 0 20px #ffff00,
                0 0 40px #ffff00;
            margin: 30px 0;
            font-weight: 900;
        }
        
        .date-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin: 25px 0;
        }
        
        .date-item {
            background: rgba(0,0,0,0.7);
            padding: 15px;
            border-radius: 10px;
            border: 1px solid #333;
        }
        
        .scanner-sweep {
            height: 3px;
            background: linear-gradient(90deg, transparent, #00ffff, transparent);
            animation: sweep 3s linear infinite;
            margin-top: 20px;
        }
        
        @keyframes sweep {
            0% { transform: translateX(-100%); }
            100% { transform: translateX(100%); }
        }
        
        .reactor-core {
            position: relative;
            text-align: center;
        }
        
        .core-animation {
            width: 100px;
            height: 100px;
            background: radial-gradient(circle, #ff0000, #660000);
            border-radius: 50%;
            margin: 20px auto;
            animation: reactor-pulse 2s ease-in-out infinite;
            position: relative;
        }
        
        .core-animation::before {
            content: '';
            position: absolute;
            top: -20px;
            left: -20px;
            right: -20px;
            bottom: -20px;
            border: 2px solid #ff0000;
            border-radius: 50%;
            animation: core-ring 3s linear infinite;
        }
        
        @keyframes reactor-pulse {
            0%, 100% { 
                transform: scale(1);
                box-shadow: 0 0 20px #ff0000;
            }
            50% { 
                transform: scale(1.1);
                box-shadow: 0 0 40px #ff0000, 0 0 60px #ff0000;
            }
        }
        
        @keyframes core-ring {
            0% { transform: rotate(0deg); opacity: 1; }
            100% { transform: rotate(360deg); opacity: 0.7; }
        }
        
        .system-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin: 30px 0;
        }
        
        .system-panel {
            background: linear-gradient(145deg, rgba(0,0,0,0.8), rgba(50,50,50,0.6));
            border: 2px solid #ff6600;
            border-radius: 10px;
            padding: 20px;
            text-align: center;
        }
        
        .panel-header {
            font-size: 0.9em;
            color: #ff6600;
            margin-bottom: 10px;
            font-weight: bold;
        }
        
        .panel-content {
            font-size: 1.1em;
            color: #ffffff;
            margin: 15px 0;
        }
        
        .power-bar {
            width: 100%;
            height: 8px;
            background: rgba(0,0,0,0.8);
            border-radius: 4px;
            overflow: hidden;
            margin-top: 10px;
        }
        
        .power-fill {
            height: 100%;
            background: linear-gradient(90deg, #00ff00, #ffff00, #ff0000);
            transition: width 2s ease;
            animation: power-flow 3s linear infinite;
        }
        
        @keyframes power-flow {
            0% { filter: brightness(1); }
            50% { filter: brightness(1.3); }
            100% { filter: brightness(1); }
        }
        
        .operational {
            color: #00ff00;
            font-weight: bold;
            font-size: 1.2em;
            text-shadow: 0 0 10px #00ff00;
        }
        
        .dossier-header {
            text-align: center;
            margin-bottom: 25px;
        }
        
        .classification {
            background: #ff0000;
            color: #ffffff;
            padding: 8px 20px;
            border-radius: 20px;
            display: inline-block;
            font-weight: bold;
            margin-top: 10px;
            animation: classification-blink 2s infinite;
        }
        
        @keyframes classification-blink {
            0%, 50% { opacity: 1; }
            51%, 100% { opacity: 0.7; }
        }
        
        .character-profile {
            display: grid;
            grid-template-columns: 150px 1fr;
            gap: 30px;
            margin: 25px 0;
        }
        
        .hologram-portrait {
            width: 150px;
            height: 150px;
            background: linear-gradient(45deg, rgba(0,255,255,0.3), rgba(0,100,255,0.3));
            border: 2px solid #00ffff;
            border-radius: 10px;
            display: flex;
            align-items: center;
            justify-content: center;
            color: #00ffff;
            font-weight: bold;
            animation: portrait-flicker 4s ease-in-out infinite;
            font-size: 0.8em;
            text-align: center;
        }
        
        @keyframes portrait-flicker {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.8; }
        }
        
        .character-name {
            font-size: 2em;
            color: #ffff00;
            margin-bottom: 20px;
            text-shadow: 0 0 15px #ffff00;
        }
        
        .data-line {
            display: flex;
            margin: 12px 0;
            padding: 8px 0;
            border-bottom: 1px solid rgba(255,255,255,0.2);
        }
        
        .data-label {
            flex: 0 0 180px;
            color: #aaaaaa;
            font-weight: bold;
        }
        
        .data-value {
            flex: 1;
            color: #ffffff;
        }
        
        .threat-assessment {
            text-align: center;
            margin-top: 20px;
            padding: 15px;
            background: rgba(255,0,0,0.2);
            border-radius: 10px;
        }
        
        .threat-high {
            color: #ff0000;
            font-weight: bold;
            text-shadow: 0 0 10px #ff0000;
        }
        
        .scan-interface {
            display: grid;
            grid-template-columns: 200px 1fr;
            gap: 30px;
            margin: 20px 0;
        }
        
        .planet-visual {
            position: relative;
            width: 180px;
            height: 180px;
            margin: 0 auto;
        }
        
        .planet-surface {
            width: 100%;
            height: 100%;
            border-radius: 50%;
            position: relative;
            animation: planet-rotate 20s linear infinite;
        }
        
        .planet-surface.tatooine {
            background: radial-gradient(circle at 30% 30%, #ffcc66, #cc9933, #996600);
            box-shadow: inset -20px -20px 40px rgba(0,0,0,0.4);
        }
        
        .planet-surface.coruscant {
            background: radial-gradient(circle at 30% 30%, #666666, #333333, #111111);
            box-shadow: inset -20px -20px 40px rgba(0,0,0,0.6);
        }
        
        .city-lights {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            border-radius: 50%;
            background-image: 
                radial-gradient(2px 2px at 30% 40%, #ffff00, transparent),
                radial-gradient(1px 1px at 60% 20%, #ffffff, transparent),
                radial-gradient(2px 2px at 80% 70%, #00ffff, transparent);
            animation: city-twinkle 3s ease-in-out infinite;
        }
        
        @keyframes city-twinkle {
            0%, 100% { opacity: 0.8; }
            50% { opacity: 1; }
        }
        
        .orbit-ring {
            position: absolute;
            top: -20px;
            left: -20px;
            right: -20px;
            bottom: -20px;
            border: 2px solid rgba(255,255,255,0.3);
            border-radius: 50%;
            animation: orbit 15s linear infinite;
        }
        
        @keyframes planet-rotate {
            from { transform: rotateY(0deg); }
            to { transform: rotateY(360deg); }
        }
        
        @keyframes orbit {
            from { transform: rotateZ(0deg); }
            to { transform: rotateZ(360deg); }
        }
        
        .scan-data {
            padding: 10px;
        }
        
        .scan-result {
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            border-bottom: 1px solid rgba(255,255,255,0.2);
        }
        
        .param {
            font-weight: bold;
            color: #aaaaaa;
        }
        
        .value {
            color: #ffffff;
        }
        
        .value.warning {
            color: #ffff00;
        }
        
        .value.danger {
            color: #ff0000;
        }
        
        .value.imperial {
            color: #ff0000;
            font-weight: bold;
        }
        
        .strategic-note {
            margin-top: 20px;
            padding: 15px;
            background: rgba(0,255,255,0.1);
            border-radius: 8px;
            font-style: italic;
        }
        
        .strategic-note.imperial {
            background: rgba(255,0,0,0.2);
            color: #ff0000;
            font-weight: bold;
        }
        
        .scanning-display {
            background: linear-gradient(145deg, rgba(255,255,0,0.1), rgba(255,150,0,0.1));
            border: 2px solid #ffff00;
            text-align: center;
        }
        
        .scan-target {
            font-size: 2em;
            color: #ffff00;
            margin: 20px 0;
            text-shadow: 0 0 15px #ffff00;
        }
        
        .progress-bars {
            margin: 30px 0;
        }
        
        .progress-line {
            display: flex;
            align-items: center;
            margin: 15px 0;
            gap: 20px;
        }
        
        .progress-line span {
            flex: 0 0 200px;
            text-align: left;
        }
        
        .bar {
            flex: 1;
            height: 8px;
            background: rgba(0,0,0,0.6);
            border-radius: 4px;
            overflow: hidden;
        }
        
        .fill {
            height: 100%;
            background: linear-gradient(90deg, #ff0000, #ffff00, #00ff00);
            transition: width 3s ease;
        }
        
        .welcome-terminal {
            background: linear-gradient(145deg, rgba(0,255,65,0.2), rgba(0,150,50,0.1));
            border: 2px solid #00ff41;
            text-align: center;
        }
        
        .terminal-boot {
            font-family: 'Courier New', monospace;
        }
        
        .boot-sequence {
            margin: 25px 0;
            text-align: left;
        }
        
        .boot-line {
            margin: 8px 0;
            opacity: 0;
            animation: boot-text 0.5s ease forwards;
        }
        
        .boot-line:nth-child(1) { animation-delay: 0.5s; }
        .boot-line:nth-child(2) { animation-delay: 1s; }
        .boot-line:nth-child(3) { animation-delay: 1.5s; }
        .boot-line:nth-child(4) { animation-delay: 2s; }
        
        @keyframes boot-text {
            to { opacity: 1; }
        }
        
        .quote-display {
            margin-top: 30px;
            padding: 20px;
            background: rgba(0,0,0,0.5);
            border-radius: 10px;
            border-left: 4px solid #ffff00;
        }
        
        .quote {
            font-style: italic;
            color: #ffff00;
            font-size: 1.2em;
        }
        
        .error-display, .sith-error {
            background: rgba(255,0,0,0.3);
            border: 2px solid #ff0000;
            color: #ff0000;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            text-align: center;
        }
        
        .static-effect {
            height: 20px;
            background: linear-gradient(90deg, 
                transparent 0%, #ff0000 25%, transparent 50%, #ff0000 75%, transparent 100%);
            animation: static 0.5s linear infinite;
        }
        
        @keyframes static {
            0% { transform: translateX(-100%); }
            100% { transform: translateX(100%); }
        }
        
        .command-section {
            background: rgba(0,0,0,0.8);
            border: 2px solid #ffff00;
            border-radius: 15px;
            padding: 30px;
            margin: 30px 0;
            position: relative;
        }
        
        .command-section::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 4px;
            background: linear-gradient(90deg, #ffff00, #ff6600, #ffff00);
            animation: command-glow 2s linear infinite;
        }
        
        @keyframes command-glow {
            0% { transform: translateX(-100%); }
            100% { transform: translateX(100%); }
        }
        
        .command-buttons {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 20px;
            margin: 30px 0;
        }
        
        .imperial-btn {
            background: linear-gradient(145deg, rgba(0,0,0,0.9), rgba(50,50,50,0.7));
            border: 2px solid #ff6b35;
            color: #ff6b35;
            padding: 18px 25px;
            text-decoration: none;
            border-radius: 12px;
            font-family: 'Orbitron', monospace;
            font-weight: bold;
            text-align: center;
            transition: all 0.4s ease;
            position: relative;
            overflow: hidden;
            text-transform: uppercase;
        }
        
        .imperial-btn::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255,107,53,0.4), transparent);
            transition: left 0.6s;
        }
        
        .imperial-btn:hover::before {
            left: 100%;
        }
        
        .imperial-btn:hover {
            background: linear-gradient(145deg, #ff6b35, #ff4500);
            color: #000;
            box-shadow: 
                0 0 30px rgba(255,107,53,0.6),
                0 0 50px rgba(255,107,53,0.4);
            transform: translateY(-3px) scale(1.02);
        }
        
        .form-section {
            background: rgba(0,0,0,0.9);
            border: 2px solid #00ff41;
            border-radius: 15px;
            padding: 30px;
            margin: 30px 0;
        }
        
        .form-control {
            width: 100%;
            max-width: 600px;
            padding: 18px;
            background: rgba(0,0,0,0.8);
            border: 2px solid #00ff41;
            color: #00ff41;
            border-radius: 8px;
            font-family: 'Orbitron', monospace;
            margin: 15px 0;
            font-size: 16px;
            transition: all 0.3s ease;
        }
        
        .form-control:focus {
            outline: none;
            border-color: #ffff00;
            box-shadow: 0 0 20px rgba(255,255,0,0.5);
            color: #ffff00;
            transform: scale(1.02);
        }
        
        .transmit-btn {
            background: linear-gradient(145deg, #ff0000, #cc0000);
            border: 3px solid #ff0000;
            color: #fff;
            padding: 18px 35px;
            font-family: 'Orbitron', monospace;
            font-weight: bold;
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.4s ease;
            box-shadow: 0 0 25px rgba(255,0,0,0.4);
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .transmit-btn:hover {
            background: linear-gradient(145deg, #ffff00, #ff6600);
            color: #000;
            box-shadow: 
                0 0 40px rgba(255,255,0,0.7),
                0 0 60px rgba(255,255,0,0.5);
            transform: scale(1.1) translateY(-2px);
        }
        
        .diagnostic-panel {
            background: rgba(0,0,0,0.95);
            border: 2px solid #666;
            padding: 25px;
            margin: 25px 0;
            border-radius: 12px;
            font-size: 0.95em;
            color: #888;
        }
        
        .diagnostic-panel h4 {
            color: #ff0000;
            margin-bottom: 15px;
            font-size: 1.1em;
        }
        
        /* Responsive design */
        @media (max-width: 768px) {
            .main-container {
                margin: 10px;
                padding: 20px;
            }
            
            .main-title {
                font-size: 2.2em;
            }
            
            .scan-interface {
                grid-template-columns: 1fr;
                text-align: center;
            }
            
            .character-profile {
                grid-template-columns: 1fr;
                text-align: center;
            }
            
            .command-buttons {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="main-container">
        <h1 class="main-title">TERMINAL IMPÉRIAL</h1>
        <h2 class="subtitle">Réseau HoloNet - Accès Sécurisé</h2>
        
EOF

echo "        $RESPONSE"

cat << 'EOF'
        
        <div class="form-section">
            <h3 style="color: #ff0000; text-align: center; margin-bottom: 25px;">📡 TRANSMISSION DE RAPPORT</h3>
            <p style="text-align: center; margin-bottom: 25px; color: #aaa;">Envoyez votre rapport de mission à l'Empire</p>
            <form method="POST" action="" style="display: flex; flex-direction: column; align-items: center;">
                <input type="text" name="username" placeholder="Code Agent Impérial" required class="form-control">
                <textarea name="message" rows="5" placeholder="Détails de la mission..." required class="form-control"></textarea>
                <button type="submit" class="transmit-btn">🚀 Transmettre À L'Empire</button>
            </form>
        </div>
        
        <div class="command-section">
            <h3 style="color: #00ffff; text-align: center; margin-bottom: 25px;">⚡ BASES DE DONNÉES IMPÉRIALES ⚡</h3>
            <p style="text-align: center; margin-bottom: 25px;">Accédez aux archives secrètes de l'Empire</p>
            <div class="command-buttons">
                <a href="?action=time" class="imperial-btn">⏰ Chronometres Galactiques</a>
                <a href="?action=system" class="imperial-btn">💀 État Étoile de la Mort</a>
                <a href="?action=character&character=luke" class="imperial-btn">👤 Dossier Luke Skywalker</a>
                <a href="?action=character&character=vader" class="imperial-btn">⚫ Dossier Dark Vador</a>
                <a href="?action=character&character=leia" class="imperial-btn">👑 Dossier Princesse Leia</a>
                <a href="?action=character&character=obiwan" class="imperial-btn">🧙 Dossier Obi-Wan</a>
                <a href="?action=character&character=jon" class="imperial-btn">👨🏻‍🦲 Dossier jveirman</a>
                <a href="?action=character&character=ed" class="imperial-btn">🍺 Dossier eschmitz</a>
                <a href="?action=character&character=sim" class="imperial-btn">🥸 Dossier slangero</a>
                <a href="?action=planet&planet=tatooine" class="imperial-btn">🏜️ Archives Tatooine</a>
                <a href="?action=planet&planet=coruscant" class="imperial-btn">🌆 Archives Coruscant</a>
            </div>
        </div>
        
        <div class="diagnostic-panel">
            <h4>🔧 DIAGNOSTICS SYSTÈME IMPÉRIAL</h4>
EOF

echo "            <p><strong>Méthode de Requête:</strong> $REQUEST_METHOD</p>"
echo "            <p><strong>Paramètres:</strong> ${QUERY_STRING:-\"Aucun\"}</p>"
echo "            <p><strong>Heure Système:</strong> $(date '+%d/%m/%Y - %H:%M:%S')</p>"
echo "            <p><strong>Port Impérial:</strong> ${SERVER_PORT:-\"Classifié\"}</p>"
echo "            <p><strong>Terminal d'Accès:</strong> ${SCRIPT_NAME:-\"Death Star Command\"}</p>"
echo "            <p><strong>Statut:</strong> <span style='color: #00ff00;'>TOUS SYSTÈMES OPÉRATIONNELS</span></p>"

cat << 'EOF'
        </div>
    </div>
    
    <script>
        // Add dynamic stars
        function createStars() {
            const numberOfStars = 100;
            for (let i = 0; i < numberOfStars; i++) {
                const star = document.createElement('div');
                star.style.position = 'fixed';
                star.style.left = Math.random() * 100 + '%';
                star.style.top = Math.random() * 100 + '%';
                star.style.width = Math.random() * 2 + 1 + 'px';
                star.style.height = star.style.width;
                star.style.background = 'white';
                star.style.borderRadius = '50%';
                star.style.opacity = Math.random();
                star.style.animation = `twinkle ${Math.random() * 3 + 2}s infinite`;
                star.style.zIndex = '-1';
                document.body.appendChild(star);
            }
        }
        
        // Add twinkle animation
        const style = document.createElement('style');
        style.textContent = `
            @keyframes twinkle {
                0%, 100% { opacity: 0.3; }
                50% { opacity: 1; }
            }
        `;
        document.head.appendChild(style);
        
        createStars();
    </script>
</body>
</html>
EOF
