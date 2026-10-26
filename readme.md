# omsyStackCtl

**Omsy Daemon & Omsy Control** : un démon (`omsyd`) et un outil en ligne de commande (`omsyctl`) pour démarrer, arrêter et superviser les services d'une stack Omsy.

> ⚠️ Code généré avec Claude, relecture humaine en cours.

## Sommaire

- [Dépendances](#dépendances)
- [Fonctionnalités](#fonctionnalités)
- [Prérequis](#prérequis)
- [Installation](#installation)
- [Configuration](#configuration)
- [Utilisation](#utilisation)
- [Structure du projet](#structure-du-projet)
- [Contribuer](#contribuer)
- [Licence](#licence)

## Dépendances

omsyStackCtl **ne fonctionne pas seul** : il pilote des services installés par deux autres projets de floagg-Studio.

| Projet | Rôle |
|--------|------|
| [OmsyServerStore](https://github.com/floagg-Studio/OmsyServerStore) | Scripts shell qui compilent et installent chaque composant depuis les sources (OpenSSL, MySQL, Apache, Nginx, PHP, Redis…) |
| [OmsyServer](https://github.com/floagg-Studio/OmsyServer) | Prépare l'environnement (utilisateur `omsy`, arborescence, dépendances), lance l'installation et décrit les services dans `systemcontroler.json` |

```
OmsyServerStore  →  OmsyServer  →  omsyStackCtl
(scripts d'install)  (environnement)  (démon + contrôle)
```

Il faut donc **installer OmsyServer (avec OmsyServerStore) avant** d'utiliser omsyStackCtl. Les dossiers référencés dans `omsy.conf` (`/chemin/vers/OmsyServer/share/logs`, `/chemin/vers/OmsyServer/share/config`, etc.) sont ceux créés par OmsyServer.

## Fonctionnalités

- Démon `omsyd` qui tourne en arrière-plan et gère vos services
- CLI `omsyctl` pour démarrer / arrêter un ou plusieurs services à la fois
- Configuration centralisée dans un fichier `omsy.conf`
- <à compléter : logs, fichiers PID, etc.>

## Prérequis

- **OmsyServer** et **OmsyServerStore** installés (voir [Dépendances](#dépendances))
- macOS (Homebrew) ou Ubuntu, comme les deux projets ci-dessus
- <à compléter : langage / runtime de omsyStackCtl et version>

## Installation

### 1. Installer OmsyServer et OmsyServerStore

```bash
git clone https://github.com/floagg-Studio/OmsyServerStore.git
git clone https://github.com/floagg-Studio/OmsyServer.git
cd OmsyServer
```

Suivez ensuite le README d'OmsyServer :

1. Adapter la variable `PATH_ROOT` dans `omsy.sh` et `install.sh`
2. `sudo bash omsy.sh` pour préparer le système
3. `bash install.sh` pour installer les composants

> Ces scripts modifient le système (paquets, utilisateurs, fichiers). Lisez-les avant de les exécuter et testez-les d'abord sur une machine virtuelle.

### 2. Installer omsyStackCtl

```bash
git clone https://github.com/is-omini/omsyStackCtl.git
cd omsyStackCtl
gcc src/omsyd.c src/common.c -o build/omsyd
gcc src/omsyctl.c src/common.c -o build/omsyctl
```

## Configuration

Le comportement est défini dans `omsy.conf`. Les chemins doivent pointer vers le dossier d'installation d'OmsyServer (`PATH_ROOT`).

| Clé       | Description                                  |
|-----------|----------------------------------------------|
| `path`    | Dossier racine de votre installation OmsyServer |
| `log`     | Dossier des fichiers de logs                 |
| `run`     | Dossier des fichiers d'exécution (PID, etc.) |
| `conf`    | Dossier de configuration d'omsyctl           |
| `confdir` | Dossier des fichiers de services             |

Exemple :

```ini
path="/chemin/vers/OmsyServer"
log="/chemin/vers/OmsyServer/share/logs/omsy"
run="/chemin/vers/OmsyServer/share/run"
conf="/chemin/vers/OmsyServer/share/config/omsyctl"
confdir="/chemin/vers/OmsyServer/share/config/omsyctl"
```

### Définir un service

Les services sont déclarés dans `/chemin/vers/OmsyServer/share/config/omsy/`. Les services déjà décrits par OmsyServer dans `systemcontroler.json` (mysql, nginx, apache, phpfpm85, redis, vsftpd) sont disponibles.

```
label="MySQL / MariaDB"
shell="/chemin/vers/OmsyServer/source/mysql.sh"
bins="mysqld","mariadbd","mysql"
start="/chemin/vers/OmsyServer/sys/bin/mysqld --defaults-file=/chemin/vers/OmsyServer/share/config/mysql/my.cnf --user=root"
stop="pkill -TERM mysqld_safe", "/chemin/vers/OmsyServer/sys/bin/mysqladmin --socket=/chemin/vers/OmsyServer/share/run/mysql.sock -u root shutdown"
label="MySQL / MariaDB"
shell="/chemin/vers/OmsyServer/source/mysql.sh"
bins="mysqld","mariadbd","mysql"
start="/chemin/vers/OmsyServer/sys/bin/mysqld --defaults-file=/chemin/vers/OmsyServer/share/config/mysql/my.cnf --user=root"
stop="pkill -TERM mysqld_safe", "/chemin/vers/OmsyServer/sys/bin/mysqladmin --socket=/chemin/vers/OmsyServer/share/run/mysql.sock -u root shutdown"
```

## Utilisation

### 1. Lancer le démon

```bash
./omsyd omsy.conf
```

### 2. Contrôler les services

```bash
./omsyctl omsy.conf start service1 [service2 service3 ...]
./omsyctl omsy.conf stop  service1 [service2 service3 ...]
```

| Commande | Effet                         |
|----------|-------------------------------|
| `start`  | Démarre les services indiqués |
| `stop`   | Arrête les services indiqués  |
| `status`   | Status des services indiqués  |

### Exemple complet

```bash
./omsyd omsy.conf
./omsyctl omsy.conf start mysql nginx phpfpm85
./omsyctl omsy.conf stop nginx
```

## Structure du projet

```
omsyStackCtl/
├── handler/     # <à compléter>
├── src/         # <à compléter>
├── omsy.conf    # Configuration principale
└── readme.md
```

## Contribuer

Les contributions sont les bienvenues : ouvrez une issue ou une pull request.