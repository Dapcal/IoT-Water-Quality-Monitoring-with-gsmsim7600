# Système IoT de Monitoring Environnemental : Qualité de l'Eau


## 📋 Présentation Générale
Ce projet consiste en une station de mesure autonome et connectée, dédiée à la surveillance en temps réel de paramètres physico-chimiques de l'eau. Conçu sur une architecture Arduino, le système intègre une gestion intelligente des capteurs, une compensation thermique dynamique et une double connectivité (Cloud et Cellulaire).

Le dispositif est particulièrement adapté pour le suivi de la potabilité, la gestion de bassins aquacoles ou le monitoring de rejets industriels dans des zones à couverture réseau limitée.

---

## 🚩 État d'Avancement et Situation
* **Situation actuelle** : Prototype fonctionnel testé en environnement contrôlé.
* **Dernières avancées** : 
    - Intégration du module **SIM7600G** pour une transmission robuste.
    - Implémentation d'un algorithme de **compensation de température** pour le pH, l'EC et le TDS.
    - Automatisation de la gestion du débit avec stockage cumulé du volume.
* **Prochaines étapes** : Optimisation de la consommation (Mode Deep Sleep), design d'un PCB dédié et sécurisation des flux de données (Architecture Zero Trust).

---

## 🚀 Fonctionnalités Techniques

### 1. Acquisition Multi-paramétrique
Le système traite simultanément sept indicateurs critiques :
- **pH** : Analyse de l'acidité/alcalinité.
- **Conductivité Électrique (EC)** : Mesure de la concentration ionique.
- **TDS (Total Dissolved Solids)** : Évaluation de la minéralisation.
- **Oxygène Dissous (DO)** : Indicateur de santé biologique de l'eau.
- **Turbidité** : Mesure de la clarté de l'eau (NTU).
- **Température** : Sonde DS18B20 pour le suivi thermique et la compensation.
- **Débit (Flow)** : Mesure du flux instantané et du volume total écoulé.

### 2. Traitement et Précision
Le code inclut une logique de **compensation thermique**. Les mesures de pH et de conductivité étant sensibles à la température, le système ajuste les valeurs brutes en temps réel pour garantir une précision métrologique constante.

### 3. Connectivité et Alertes
- **Transmission GPRS** : Envoi structuré des données vers **ThingSpeak** via des requêtes HTTP (intervalle actuel : 15 minutes).
- **Redondance SMS** : Envoi automatique de rapports SMS détaillés à des numéros administratifs prédéfinis.

---

## 🛠️ Spécifications Matérielles (Pinout)

| Composant | Broche Arduino | Rôle |
| :--- | :--- | :--- |
| **SIM7600G** | D7 (RX) / D8 (TX) | Communication cellulaire |
| **Capteur pH** | A6 | Lecture analogique |
| **Capteur EC** | A7 / A8 (Temp) | Conductivité & Température dédiée |
| **Capteur TDS** | A9 | Analyse de minéralisation |
| **Oxygène Dissous** | A10 | Niveau d'O2 |
| **Turbidité** | A11 | Clarté de l'eau |
| **Débitmètre** | D3 | Interruption matérielle (Impulsions) |
| **DS18B20** | D5 | Température précise (OneWire) |

---

## 💻 Installation et Déploiement

### Dépendances
Les bibliothèques suivantes sont requises pour la compilation :
- `SoftwareSerial` (Communication GSM)
- `DFRobot_PH` & `DFRobot_ECPRO` (Modules DFRobot)
- `OneWire` & `DallasTemperature` (Sonde thermique)
- `EEPROM` (Stockage des calibrations)

### Configuration
1. Clonez le dépôt : `git clone https://github.com/Dapcal/IoT-Water-Quality-Monitoring-with-gsmsim7600.git`
2. Dans le fichier `.ino`, configurez l'APN de votre opérateur (ex: `orange.com`).
3. Renseignez votre clé API ThingSpeak et les numéros de téléphone cibles.
4. Téléversez sur votre carte Arduino.

---

## 🛡️ Perspectives Sécurité
- **Accès Distant** : Filtrage des numéros entrants pour le contrôle du module par SMS.

---

## 📝 Auteur
**BICABA Dapoba Calixte Igor** 
