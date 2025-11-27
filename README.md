# ResaLab
![Status](https://img.shields.io/badge/status-en_développement-orange)
![ESP32-C3](https://img.shields.io/badge/ESP32--C3-Expressif-blue)
![E--Ink](https://img.shields.io/badge/Display-E--Ink-black)
![PCB](https://img.shields.io/badge/Hardware-PCB--A-green)
---
**ResaLab** est un projet multidomaine ayant pour objectif de **simplifier et centraliser la procédure de réservation des ressources du FabLab**.  
Il vise à offrir une plateforme fiable, intuitive et interconnectée entre le site web, la base de données et les équipements physiques du FabLab.

## ⚙️ Configuration requise

Certaines constantes doivent être définies dans le firmware :

```
const char* deviceId = "00000001";  // Id du device enregistré sur le serveur
```

Un fichier **identifier.h** doit être créé et contenir :

```
const char* ssid = "";
const char* password = "";
const String serverUrl = "";
```

Ce fichier doit être inclus dans le code principal.

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
## ⚙️ Contraintes techniques

La solution matérielle repose sur une carte électronique (PCB-A) spécialement conçue pour accueillir l’ensemble des composants nécessaires au fonctionnement du système ResaLab.  
Les principales contraintes techniques sont les suivantes :

### 🔌 Alimentation
- La PCB est alimentée en **5V**.  
- Une conversion est effectuée vers du **3.3V / 3A**, tension nécessaire l’écran e-ink et les autres composants basse tension.  
- La capacité de **3A** permet de gérer les pics de consommation liés aux LED haute puissance et au module Wi-Fi.

### 💡 Gestion des LED 3W
- La carte comporte des pads **TP3 à TP8** destinés au câblage de **trois LED de 3W**.  
- Ces LED sont contrôlées via des transistors (**Q1, Q2, Q3**) alimentés par le rail 3.3V / 3A.  
- Les couleurs prévues sont :
  - 🔴 Rouge  
  - 🟠 Orange  
  - 🟢 Vert  
- Ces LED servent principalement aux retours visuels (disponible / réservé / en cours d’utilisation).
- GPIO LEDS: green = 0;  orange=9;  red=1.

### 🔘 Boutons lumineux
- La PCB inclut des pads **TP9 à TP17** pour souder **trois boutons lumineux**.  
- Chaque bouton dispose de pads dédiés :
  - **GND**  
  - **VCC**  
  - Signal de lecture  
- Les boutons servent à l’interaction utilisateur autour de la validation ou navigation dans l’interface locale.

### 🧠 Module ESP32-C3
- La carte est conçue pour accueillir un **ESP32-C3 DevKit M**.  
- Ce module assure :
  - la connexion Wi-Fi sécurisée  
  - la communication avec le serveur  
  - la gestion des LED, boutons et écran

### 🖥️ Interface écran e-ink (SPI 4 lignes)
- La PCB expose des pins dédiés pour connecter un écran **e-ink 2.9"**, via un bus **SPI 4 lignes**.  
- Le connecteur est prévu pour être compatible avec un câblage via **dupont wires**.
- Mosi: 7
- SCLK: 6
- CS: 10
- DC: 8
- RST: 3
- BUSY: 2

### 🧩 GPIO supplémentaires
- Les derniers GPIO non utilisés sont exposés via un **dernier connecteur dupont (J3)**.  
- Ce connecteur permet d’ajouter des périphériques supplémentaires ou d'effectuer des tests.



