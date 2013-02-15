Heimautomatisierungssoftware:
=============================
Eine Plugin orientierte Steuersoftware für die Heimautomatisierung. Alle weiteren Informationen, die Roadmap
und Downloads befinden sich auf der [Webseite](http://davidgraeff.github.com/scenecontrol)  dieses Projekts.

Code Stabilität: [![Build Status](https://travis-ci.org/davidgraeff/scenecontrol.png?branch=master)](https://travis-ci.org/davidgraeff/scenecontrol)

Anwendungsgebiete:
==================
* Intelligenter Wecker
* Energiesparsystem
* Intelligenter Raum
* Automatisiertes Heimkino

Grafisches Oberfläche für Scene Control:
========================================
Um Ereignisse, Bedingungen und Aktionen in Szenen grafisch zu organisieren und zu erstellen befindet
sich eine Web-Anwendung im Ordner "htmleditor".

Android-App:
============
Einige Plugins, sowie die Basisfunktionalität des SceneServers können von einer
Android App gesteuert werden.

Abhängigkeiten:
---------------
| Name        | Ubuntu Paket           | Beschreibung  | Komponenten
| ------------- |:-------------:| -----:| -----|
| Qt4      | libqt4-dev | Qt4 Framework | Kernprozess
| SSL      | libssl-dev | Sichere Verbindung | Kernprozess, Kontrollsocket
| PAM      | libpam0g-dev      |   Benutzerauthentifizierung | SessionProxy
| Pulseaudio | libpulse-dev      |    Pulseaudio | Plugin: Pulseaudio
| UDEV | libudev-dev      |    Linux Input Events | Plugin: Linux_input_events
