// -----------------------------------------------------------
// CONFIGURATION DU TAUX DE VICTOIRE (0.05 = 5%, 1.0 = 100%)
var WIN_RATE = 0.05; 
// -----------------------------------------------------------

// NOTE POUR LES ADMINS:
// IP DE L'ESP32 (Serveur Goodies) : 192.168.4.1

// -----------------------------------------------------------

const symbols = ['🍒', '🍋', '🍊', '🍇', '🍉', '🍓', '💎', '7️⃣'];
const socket = new WebSocket('ws://localhost:8765'); // Connexion au pont Python
let isSpinning = false;

// --- GESTION DU RÉSEAU ---
socket.onopen = function(e) {
    console.log("Connecté au système central Python.");
    // Petit effet visuel pour dire qu'on est connecté
    document.getElementById('message').innerText = "SYSTEME CONNECTÉ";
    setTimeout(() => { document.getElementById('message').innerText = ""; }, 2000);
};

socket.onmessage = function(event) {
    console.log("Message reçu:", event.data);
    const data = JSON.parse(event.data);
    
    if (data.action === "SPIN") {
        // L'ordre vient du bouton physique ESP32 -> Python -> Ici
        requestSpin();
    }
    else if (data.action === "HACK_DETECTED") {
        // Le Python a grillé le hack -> On change l'affichage
        const marq = document.getElementById("marqueeText");
        marq.innerText = "⚠️ PROBABILITÉ TRAFIQUÉE ⚠️";
        marq.className = "marquee-text hack-alert";
        
        // On affiche aussi un message de perte
        const msg = document.getElementById('message');
        msg.innerText = "ACCÈS REFUSÉ";
        msg.className = "message lose";
    }
};

// --- LOGIQUE DU JEU (HACKABLE) ---
function requestSpin() {
    if (isSpinning) return;
    isSpinning = true;
    document.getElementById('spinBtn').disabled = true;
    document.getElementById('message').innerText = "";
    document.getElementById('message').className = "message";

    // 1. DÉCISION DU RÉSULTAT (C'est ici que WIN_RATE agit)
    // On tire un nombre entre 0 et 1. Si inféreur à WIN_RATE, on gagne.
    const isWin = Math.random() < WIN_RATE;
    let finalSymbols = [];

    if (isWin) {
        // VICTOIRE FORCÉE : On choisit 1 symbole et on le met 3 fois
        const winningSymbol = symbols[Math.floor(Math.random() * symbols.length)];
        finalSymbols = [winningSymbol, winningSymbol, winningSymbol];
    } else {
        // DÉFAITE FORCÉE : On s'assure de ne pas avoir 3 identiques
        do {
            finalSymbols = [
                symbols[Math.floor(Math.random() * symbols.length)],
                symbols[Math.floor(Math.random() * symbols.length)],
                symbols[Math.floor(Math.random() * symbols.length)]
            ];
        } while (finalSymbols[0] === finalSymbols[1] && finalSymbols[1] === finalSymbols[2]);
    }

    // 2. LANCEMENT DE L'ANIMATION AVEC LE RÉSULTAT PRÉDÉFINI
    animateReels(finalSymbols, isWin);
}

async function animateReels(targets, isWin) {
    const reelContainers = [
        document.getElementById('reel1-container'),
        document.getElementById('reel2-container'),
        document.getElementById('reel3-container')
    ];

    // On fait tourner les rouleaux visuellement
    // Boucle simple pour simuler le défilement rapide
    const spins = 20; // Nombre de changements d'images
    
    for (let i = 0; i < spins; i++) {
        reelContainers.forEach(container => {
            const randomSymbol = symbols[Math.floor(Math.random() * symbols.length)];
            container.innerHTML = `<div class="reel-symbol" style="filter: blur(2px);">${randomSymbol}</div>`;
        });
        // On ralentit progressivement
        await new Promise(r => setTimeout(r, 50 + (i * 10)));
    }

    // Affichage du résultat final (celui calculé par le hack)
    targets.forEach((symbol, index) => {
        const container = reelContainers[index];
        container.innerHTML = `<div class="reel-symbol" style="transform: scale(1.2); transition: 0.2s;">${symbol}</div>`;
    });

    // 3. FIN DU TOUR & COMMUNICATION AVEC PYTHON
    setTimeout(() => {
        isSpinning = false;
        document.getElementById('spinBtn').disabled = false;

        if (isWin) {
            const msgEl = document.getElementById('message');
            
            if (targets[0] === '💎' || targets[0] === '7️⃣') {
                msgEl.innerText = "💎 JACKPOT !!! 💎";
                msgEl.className = "message win";
            } else {
                msgEl.innerText = "🎉 GAGNÉ ! 🎉";
                msgEl.className = "message win";
            }
            
            // ON ENVOIE LA PREUVE AU SERVEUR (Taux utilisé + Action)
            console.log("Envoi de la demande de validation au serveur...");
            socket.send(JSON.stringify({
                action: "WIN_CHECK", 
                rate: WIN_RATE
            }));

        } else {
            const msgEl = document.getElementById('message');
            msgEl.innerText = "❌ PERDU ❌";
            msgEl.className = "message lose";
        }
    }, 500);
}

// Touche Espace pour lancer
document.addEventListener('keydown', (e) => {
    if (e.code === 'Space' && !isSpinning) {
        e.preventDefault();
        requestSpin();
    }
});
