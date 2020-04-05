# PROJET SERVEUR FTP
## Alaa BEN FATMA & Wassim AYARI
---
📚 English readme will be coming soon.....

---

## Installation
### Clone
Via HTTPS

>git clone https://github.com/alaabenfatma/TinyFTP.git

Via SSH

>git clone git@github.com:alaabenfatma/TinyFTP.git

Sinon, vous pouvez télécharger le projet en tant qu'archive .zip ou faire un forkt.
---

Compilation:
>make clean

>make
---
Exécution serveur:

>./server/server
---
Exécution client:
>cd client

>./client <adresse>  (localhost en local)
---

Differents test à réaliser:

### Commande get:

>get tests/Docs/small.txt 

_(A noter: get devrait également fonctionner avec les autres type de fichier: pdf, jpg...)_


Le dossier tests contient plusieurs types de fichier (texte, image, gif, sons...), on pourra tester get sur ces fichiers.

---

### Gestion de panne:

>get tests/heavy.txt

>Ctrl-c 

>./client <adresse>
  
>resume

---
### Equilibrage de charge

Connecter plus de NPROC client. Le serveur affichera comment il affecte les clients aux differents fils.
On remarquera qu'il sera impossible pour le prochain client de se connecter jusqu'à ce qu'un serveur ce libère.

---

### Exemples avec les commandes incorporés:

>ls 

>cd tests/Docs

>pwd

>clear

>bye

---

### Commandes necessitant un login:

>mkdir mon_dossier

>rm tests/Docs/small.txt

>rm -r tests/Sound

>put nom_fichier 

_(Le fichier devra se trouver dans le répertoire client!!)_

Normalement, on remarquera qu'une erreur surviendra car vous n'êtes pas connecté.

---

### Créer un login:

>register

>mon_username

>mon_mdp

---

### Se connecter:
>login

>mon_username

>mon_mdp

---

### Se déconnecter:
>login

>mon_username

>mon_mdp

On pourra maintenant tester les commandes mkdir, rm et rm -r
