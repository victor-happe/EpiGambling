import asyncio
import websockets
import serial
import serial.tools.list_ports
import json

# --- CONFIGURATION ---
SERIAL_PORT = 'COM5' # Remplace avec ton port (ex '/dev/tty.usbserial-0001' for MacOs)
BAUD_RATE = 115200

# La "Vraie" valeur. Si le client envoie autre chose, c'est un hack.
OFFICIAL_WIN_RATE = 0.05 

# --- ETAT DU SYSTEME ---
connected_clients = set()
esp_serial = None
hacker_wins = 0  # Compteur de victoires en mode triche

async def serial_listener():
    """Écoute le bouton de l'ESP32"""
    global esp_serial
    print(f"🔌 Connexion série sur {SERIAL_PORT}...")
    try:
        esp_serial = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
        print("✅ ESP32 Connecté USB")
        # Reset all motors to neutral/safe position on connection
        try:
            esp_serial.write(b"RESET_NEUTRAL\n")
            print("🔄 Sent RESET_NEUTRAL to ESP32")
        except Exception as e:
            print(f"❌ Erreur en envoyant RESET_NEUTRAL: {e}")
    except Exception as e:
        print(f"❌ Erreur Série: {e}")
        return

    while True:
        if esp_serial and esp_serial.in_waiting > 0:
            try:
                line = esp_serial.readline().decode('utf-8').strip()
                if line == "BUTTON_PRESSED":
                    print("🔘 Bouton Physique -> Lancement du Spin")
                    if connected_clients:
                        # On dit à toutes les pages web de lancer la roue
                        msg = json.dumps({"action": "SPIN"})
                        await asyncio.gather(*[client.send(msg) for client in connected_clients])
            except Exception:
                pass
        await asyncio.sleep(0.01)

async def websocket_handler(websocket):
    global hacker_wins
    print("💻 Client Web connecté")
    connected_clients.add(websocket)
    
    try:
        async for message in websocket:
            data = json.loads(message)
            
            # Le client nous dit qu'il a gagné, et nous donne son taux de win utilisé
            if data.get("action") == "WIN_CHECK":
                client_rate = data.get("rate")
                
                print(f"🔍 Vérification... Taux client: {client_rate} | Officiel: {OFFICIAL_WIN_RATE}")

                # 1. EST-CE UN HACK ? (Le taux est différent)
                if abs(client_rate - OFFICIAL_WIN_RATE) > 0.001: 
                    print("⚠️ HACK DETECTÉ !")
                    
                    if hacker_wins >= 1:
                        print("🚫 TROP DE GAGNE : Blocage goodies + Alerte")
                        # On n'active PAS le servo
                        # On envoie l'ordre de changer le texte
                        alert_msg = json.dumps({"action": "HACK_DETECTED"})
                        await websocket.send(alert_msg)
                    else:
                        print("🔓 PREMIER HACK : Goodies autorisé (1 seule fois)")
                        hacker_wins += 1
                        if esp_serial: esp_serial.write(b"WIN_TRIGGER\n")
                
                # 2. JOUEUR LEGIT
                else:
                    print("✅ Joueur Legit : Goodies autorisé")
                    if esp_serial: esp_serial.write(b"WIN_TRIGGER\n")

    except Exception as e:
        print(f"Déconnexion: {e}")
    finally:
        connected_clients.remove(websocket)

async def main():
    print("🚀 Serveur Anti-Cheat démarré sur ws://localhost:8765")
    async with websockets.serve(websocket_handler, "localhost", 8765):
        await serial_listener()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Arrêt.")