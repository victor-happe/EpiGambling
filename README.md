# Guide utilisateur
### Prérequis:
- Une prise mural vers USB-B
- Un câble USB-B vers USB-C
- Un ordinateur avec **Python 3.7+** installé
### Mise en place:
##### Branchement
Branchez l'embout USB-B à la prise mural, et l'ordinateur à la carte esp32 via le câble USB-B vers USB-C.
##### Téléchargement
Si c'est votre première mise en place et que vous n'avez pas télécharger le __driver esp32__, veuillez le trouver [ici](https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads). Il vous faudra aussi installer python. Si vous ne l'avez pas déjà fait, le lien pour télécharger python se trouve [ici](https://www.python.org/downloads/). Téléchargez le projet en sélectionnant le bouton vert Code puis Download ZIP. Extrayez les fichiers de l'archive ZIP.
##### Configuration
Ouvrez usb-control.py et remplacez la variable SERIAL_PORT par le port connecté à la carte esp32 (voici un [tutoriel](https://www.mathworks.com/help/matlab/supportpkg/find-esp32-port-on-windows-mac-and-linux.html) si vous ne le connaissez pas).
##### Lancement
Vous pouvez désormais lancer le script python. Pour ce faire, cherchez cmd dans le recherche windows, et executez __cd 'votre path'__, puis __python usb-control.py__. Pour copier votre path, ouvrez le dossier où vous avez extrait le projet sur file explorer, puis double cliquez la barre où se trouve la "route", à coté de la barre de recherche.
Une fois le script lancé, ouvrez index.html. Vous pouvez appuyez sur le boutton pour lancer la machine.