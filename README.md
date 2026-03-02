# ResaLab
![Status](https://img.shields.io/badge/status-en_développement-orange)
![ESP32-C3](https://img.shields.io/badge/ESP32--C3-Expressif-blue)
![E--Ink](https://img.shields.io/badge/Display-E--Ink-black)
![PCB](https://img.shields.io/badge/Hardware-PCB--A-green)

---

**ResaLab** est un projet multidomaine ayant pour objectif de **simplifier et centraliser la procédure de réservation des ressources du FabLab**.  
Il vise à offrir une plateforme fiable, intuitive et interconnectée entre le site web, la base de données et les équipements physiques du FabLab.

---

# 📌 Module matériel – Interface HMI

Le module ResaLab est une **interface homme-machine (IHM)** destinée à être installée directement sur les équipements du FabLab :

- Imprimantes 3D  
- Brodeuses numériques  
- Découpeuses laser  

## 🧩 Composition du module

- 🖥️ Écran **e-ink 2.9"**
- 🔴🟠🟢 Trois LED haute puissance (3W)
- 🔘 Trois boutons lumineux
- 🧠 Module **ESP32-C3**
- ⚡ Alimentation USB-C
- 📐 PCB sur mesure (PCB-A)

---

# 🎯 Fonctionnalités principales

Le module permet :

- ✅ D’afficher l’état de la machine (Disponible / Réservée / En cours)
- 📅 De synchroniser les réservations avec le serveur
- 🔐 De valider localement la présence de l’utilisateur
- 💡 De fournir un retour visuel clair via LED haute puissance
- 🌐 De communiquer en Wi-Fi avec la plateforme centrale

---

# ⚙️ Configuration requise

Certaines constantes doivent être définies dans le firmware :

```cpp
const char* deviceId = "00000001";  // ID du device enregistré sur le serveur

Un fichier identifier.h doit être créé et contenir :

const char* ssid = "";
const char* password = "";
const String serverUrl = "";

``` 

Ce fichier doit être inclus dans le code principal.

---

## Architecture générale du projet

Le projet est divisé en plusieurs domaines complémentaires :


**Site Web**
Plateforme de réservation en ligne
Gestion des utilisateurs et des droits d'accès
Gestion des réservations (création, modification, consultation)
Base de données centralisée
Historique d’utilisation

**Gestion des transactions / crédits**
Carte électronique (ESP32-C3)
Interface Homme-Machine locale
Validation des réservations via API serveur
Autorisation d’accès à l’équipement

Affichage des informations sur écran e-ink
Pilotage des LED 3W
Communication Wi-Fi sécurisée

**Modélisation du boîtier**
Conception 3D du boîtier
Intégration ergonomique des boutons et LEDs
Adaptation aux contraintes d’impression 3D
Optimisation pour maintenance et production FabLab

Voici certaines contraintes à tenir en compte:
Alimentation
Entrée : 5V
Conversion vers 3.3V / 3A
Capacité 3A dimensionnée pour :

Pics Wi-Fi
LED haute puissance
Écran e-ink

## Gestion des LED 3W
Pads TP3 à TP8
Pilotage via transistors Q1, Q2, Q3
Couleurs :
🔴 Rouge
🟠 Orange
🟢 Vert

*GPIO LED*
green  = 0
orange = 9
red    = 1


*Boutons lumineux*
Pads TP9 à TP17



## Interface écran e-ink (SPI 4 lignes)

Écran 2.9 connecté via SPI. https://github.com/WeActStudio/WeActStudio.EpaperModule

Mapping GPIO
MOSI : 7
SCLK : 6
CS   : 10
DC   : 8
RST  : 3
BUSY : 2

Connexion prévue via dupont wires.

GPIO supplémentaires

Les GPIO restants sont exposés via le connecteur J3.
Permet d’ajouter des périphériques ou de réaliser des tests.

## État du projet

Version actuelle : PCB-A (Proof of Concept)

## État du projet

La première version de la PCB a servi de **Proof of Concept (PoC)**.

Une nouvelle révision est prévue afin de :

- Corriger les défauts de la V1
- Permettre la programmation directement via le port **USB-C d’alimentation**
- Simplifier l’intégration et améliorer la fiabilité

---

## Roadmap (prévisionnelle)

- [ ] Révision PCB V2
- [ ] Intégration module ESP32-C3
- [ ] Support complet OTA
- [ ] Intégration avec le système de réservation (backend ResaLab)

---

## Statut

Projet en développement actif.  
Version actuelle : Prototype fonctionnel (PoC).