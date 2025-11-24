Mosi: 7
SCLK: 6
CS: 10
DC: 8
RST: 3
BUSY: 2

GPIO LEDS: green = 0;  orange=9;  red=1.

# ResaLab

**ResaLab** est un projet multidomaine ayant pour objectif de **simplifier et centraliser la procédure de réservation des ressources du FabLab**.  
Il vise à offrir une plateforme fiable, intuitive et interconnectée entre le site web, la base de données et les équipements physiques du FabLab.

---

## 🎯 Objectifs du projet

- Mettre en place un système de réservation simple, rapide et sécurisé.  
- Assurer une authentification fiable des utilisateurs directement sur les machines.  
- Permettre un suivi clair des réservations, de l’utilisation des machines et de l’accès aux ressources.  
- Fournir une interface physique (HMI) intuitive via une carte électronique dédiée.  
- Proposer une solution matérielle complète : électronique + boîtier.

---

## 🛠️ Architecture générale du projet

Le projet est divisé en **trois domaines complémentaires** :

### 1. **Site Web**

- Plateforme de réservation en ligne.  
- Gestion des utilisateurs et des droits d'accès.  
- Gestion des réservations (création, consultation, modification).  
- Intégration d’une base de données centralisée.  
- Suivi de l’historique d’utilisation.  
- Gestion des transactions et éventuels crédits.

---

### 2. **Carte électronique (ESP32-C3)**

- Interface Homme-Machine (HMI) pour les utilisateurs.  
- Validation locale des réservations via connexion au serveur.  
- Allumage ou autorisation d’accès à l’équipement concerné.  
- Affichage e-ink pour les informations de réservation.  
- LED dome pour retour visuel (état réservation / disponibilité).  
- Communication sécurisée avec la plateforme en ligne.

---

### 3. **Modélisation du boîtier**

- Conception 3D du boîtier accueillant l’ESP32-C3 et l’écran e-ink.  
- Intégration ergonomique des boutons, LEDs et connecteurs.  
- Respect des contraintes mécaniques, esthétiques et d’impression 3D.  
- Optimisation pour une production FabLab (impression, assemblage, maintenance).

---



