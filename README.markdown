Raumautomatisierungsoftware:
============================
Eine Plugin orientierte Steuersoftware für die Raumautomatisierung. Plugins für
Steckdosen, Rollo, Led Beleuchtung, PulseAudio, Videoabspielprogramm,
Beamersteuerung und Zeitbasierte Ereignisse sind vorhanden.

Diese README gibt erste Hintergrundinformationen.
Die Dokumentation befindet sich unter ambiente.dyndns.dk/roomcontrol/

Unterstützte Plattformen:
=========================
* Windows ab Vista (Qt4;OpenSSL;Windows API: CryptProtectData, CryptUnprotectData, LogonUser)
* Linux ab 2.6.22 (Qt4;OpenSSL;pam)

Mitgelieferte Plugins:
======================
Generische Plugins:
-------------------
* Leds: Generisches Leuchtdioden Plugin zur Verwaltung aller angemeldeten Leuchtdioden
* Switches: Generisches Steckdosen Plugin zur Verwaltung aller angemeldeten Steckdosen
* Modes: Bietet eine frei änderbare Variable an um den Modus des Servers (etwa Musiksteuerung, Videosteuerung) zu markieren.
* Time: Bietet feste und periodische Zeitereignisse an.
* WOL: Wakeup on lan. Ermöglicht das starten eines über die angegebene MAC festgelegten PCs, sofern die Zielnetzwerkkarte dies unterstützt.

Generisches Plugins (Betriebssystemabhängig):
---------------------------------------------
* Linux Input Events (linux only): Tastenereignisse vom Linux Input Framework (etwa usb Fernbedienungen, Tastaturen).

Softwareabhängige Plugins:
--------------------------
* MPD: Steuert den Music Player Daemon.
* XBMC: Steuert das XBox Media Center.
* Pulseaudio (linux only): Steuert Lautstärkekanäle des Pulseaudio daemons.

Hardwareabhängige Plugins:
--------------------------
* Anel sockets: Steuert Steckdosenleisten von der Firma anel (http://www.anel-elektronik.de/).
* Cardreader PCSC: Reagiert auf das Ein/Ausstecken von Karten bei PCSC Kompatiblen Kartenlesegeräten.
* Projector Sanyo Z700: Steuert den Sanyo Projector Z700 über das serielle Interface.

Plugins für mitgelieferte Tools:
--------------------------------
* RemoteSystem: Ermöglicht Steuerbefehle an alle Computer im selben Subnetzwerk,
  welche die mitgelieferte Clientsoftware gestartet haben, abzusetzen.
* Roomcontrol_Leds_serial: Steuert den, über ein RS232 Anschluss angebundenen,
  Atmega µController mit der Firmware unter tools/firmware/ethersex um Leuchtdioden anzusteuern.
* Roomcontrol_Leds_udp: Steuert den, über UDP/IP angebundenen,
  Atmega µController mit der Firmware unter tools/firmware/ethersex um Leuchtdioden anzusteuern.
* Roomcontrol_curtain: Steuert den, über UDP/IP angebundenen,
  Atmega µController mit der Firmware unter tools/firmware/ethersex um ein Rollo anzusteuern.

Architektur:
============
Kernprozess:
------------
Der ausführende Kernprozess "roomcontrolserver" wird im weiteren Server genannt.
Der Server baut nach dem Start eine Verbindung zur Datenhaltung auf und fordert alle Plugin Konfigurationen an.
Eine Plugin Konfiguration enthält die benötigten Parameter um eine bestimmte Plugin Instanz
erfolgreich starten zu können. Es können also jederzeit mehrfache Instanzen eines Plugins gestartet werden.
Plugin Instanzen laufen in eigenen Prozessen und können untereinander oder mit dem Server kommunizieren.

Sobald ein Pluginprozess angelaufen und eine Verbindung zum Server hergestellt worden ist, werden aus der Datenhaltung
alle auf diese Plugin Instanz registrierten Ereignisse angefordert and an die Plugin Instanz weitergereicht.

Sobald ein Ereignis entsprechend der Ereignisdaten eintritt, wird der Server informiert und lädt aus der Datenhaltung
die mit dem Ereignisdaten verbundenen Profile und damit wiederum verbundene Bedingungsdaten und Aktionsdaten.

Wenn alle betroffenen Plugin Instanzen die Bedingungen anhand der Bedinungsdaten mit positiver Rückmeldung
evaluiert haben, werden die Aktionsdaten an die zuständigen Plugin Instanzen zur Ausführung gesendet.

Über einen TCP Port i.d.R. Port 3101 können per JSON kodierte Nachrichten an den Prozess abgesetzt werden.
Der Server selber bietet keine Session/Sicherheitsverwaltung an,
diese Funktionalitäten können mit zusätzlichen Proxy Prozessen abgedeckt werden.

Websocketsproxy + SessionProxy:
-------------------------------
Über den Websocketsproxy können sich über https Wenclients mit dem Server verbinden. Es wird eine
Authentifizierung gegenüber den auf dem Betriebssystem vorhandenen Benutzern
durchgeführt, sofern auch der SessionProxy aktiv ist. Zugriffsrechte werden dann durch Gruppenmitgliedschaften des
angegeben Benutzers geregelt.

Plugins:
--------
Plugins sind eigene Prozesse, welche selbstständig eine Kommunikationsverbindung zum Server aufbauen müssen.
Über QDataStream kodierte Nachrichten kann mit anderen Plugins oder dem Server kommuniziert werden.
Plugins können Eigenschaften (properties) besitzen und auf Eigenschaftsänderungen des Servers oder anderer Plugins
reagieren. 

Datenspeicherung:
-----------------
Welches Ereignis, unter welchen Bedinungen welche Aktion auslöst wird in einer Datenbank vorgehalten. Für diesen
Zweck wird die dokumentenbasierte Datenbank MongoDB verwendet. Über MongoDB Werkzeuge kann der Datenbestand regelmäßig
gesichert, redundant vorgehalten oder verfielfältigt werden.

Beispiel: Ereignisse, Bedingungen und Aktionen:
-----------------------------------------------
Ein Ereignis kann das Eintreten eines gewissen Zeitpunktes sein,
eine Bedingung könnte den aktuellen Steckdosenzustand meinen,
eine Aktion löst eine Veränderung aus etwa das Ändern der Lichwerte von Leuchtdioden.

Profile:
--------
Ereignisse, Bedingungen und Aktionen machen erst Sinn, sobald diese zusammengefasst werden können.
Dies geschieht durch Profile (Collections). Sobald ein Ereignis in einem Profil eintritt,
werden die Bedingungen geprüft und dann ggfs. die im Profil definierten Aktionen,
evtl. mit eingestellter zeitlicher Verzögerung, ausgeführt.


Anwendungsgebiete:
==================
1. Intelligenter Wecker
2. Energiesparsystem
3. Intelligenter Raum
4. Automatisiertes Heimkino

Clients:
========
Es befinden sich in QML/qt4 geschriebene Clientprogramme im Ordner clients/.

Benötigte Bibliotheken für den Server ohne Plugins:
===================================================
Kernprozess:
Name          Ubuntu Paket          Beschreibung
Qt4           libqt4-dev            Qt4 Framework

SessionProxy:
Name          Ubuntu Paket          Beschreibung
PAM           libpam0g-dev          Benutzerauthentifizierung

Websocketsproxy:
Name          Ubuntu Paket          Beschreibung
libwebsockets ----                  Websocket Bibliothek  (http://git.warmcat.com/cgi-bin/cgit/libwebsockets/) / LGPL 2.1
