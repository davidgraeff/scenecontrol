Heimautomatisierungssoftware:
=============================
Eine Plugin orientierte Steuersoftware für die Heimautomatisierung. Einige auf die Bedürfnisse
der Entwickler abgestimmten Plugins sowie generische Plugins sind bereits vorhanden.
Diese README gibt erste Hintergrundinformationen.

Unterstützte Plattformen:
=========================
* Windows ab Vista (Qt4;OpenSSL;Windows API: CryptProtectData, CryptUnprotectData, LogonUser)
* Linux ab 2.6.22 (Qt4;OpenSSL;pam)

Mitgelieferte Plugins:
======================
Generische Plugins:
-------------------
* Leds: Generisches Leuchtdioden Plugin zur Verwaltung aller angemeldeten Leuchtdioden
* DMX/ArtNet: Erlaubt die Steuerung von konfigurierten DMX Lampen über das Leds Plugin. Verwendet ArtNet, also ist eine Hardware Umsetzung IP-->DMX notwendig.
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
* Projector Sanyo Z700: Steuert den Sanyo Projector Z700 über das serielle Interface.

Plugins für mitgelieferte Tools:
--------------------------------
* RemoteSystem: Ermöglicht Steuerbefehle an alle Windows Computer im selben Subnetzwerk,
  welche die mitgelieferte Clientsoftware gestartet haben, abzusetzen.
  Zugehöriges Projekt: https://github.com/davidgraeff/windows_net_control

Architektur:
============
Kernprozess:
------------
Der ausführende Kernprozess wird im weiteren SceneServer genannt.
Der SceneServer baut nach dem Start eine Verbindung zur Datenhaltung auf und fordert alle Plugin Konfigurationen an.
Eine Plugin Konfiguration enthält die benötigten Parameter um eine bestimmte Plugin Instanz
erfolgreich starten zu können. Es können also jederzeit mehrfache Instanzen eines Plugins gestartet werden.
Plugin Instanzen laufen in eigenen Prozessen und können untereinander oder mit dem SceneServer kommunizieren.

Sobald ein Pluginprozess angelaufen und eine Verbindung zum Server hergestellt worden ist, werden aus der Datenhaltung
alle auf diese Plugin Instanz registrierten Ereignisse angefordert and an die Plugin Instanz weitergereicht.

Sobald ein Ereignis entsprechend der Ereignisdaten eintritt, wird der SceneServer informiert und lädt aus der Datenhaltung
die mit dem Ereignisdaten verbundenen Profile und damit wiederum verbundene Bedingungsdaten und Aktionsdaten.

Wenn alle betroffenen Plugin Instanzen die Bedingungen anhand der Bedinungsdaten mit positiver Rückmeldung
evaluiert haben, werden die Aktionsdaten an die zuständigen Plugin Instanzen zur Ausführung gesendet.

Über einen TCP Port i.d.R. Port 3101 können per JSON kodierte Nachrichten an den Prozess abgesetzt werden.
Der Server selber bietet keine Session/Sicherheitsverwaltung an und bietet außer direkter Sockets keine andere
Kontrollmöglichkeit an. Diese Funktionalitäten können mit zusätzlichen Proxy Prozessen abgedeckt werden.

Durch die Trennung in Prozesse mit fest gelegtem, eingegrenztem Aufgabenbereich ist die Ausfallsicherheit der
gesamten Architektur besonders hoch und für den 24/7 Betrieb ausgelegt.

Websocketsproxy:
----------------
Über den Websocketsproxy können sich über https Webclients über das WebSocket Protokoll mit dem SceneServer verbinden
und das JSON Kommandointerface des SceneServers nutzen.

SessionProxy:
-------------
Der SessionProxy handelt mit dem SceneServer einen neuen Port für das JSON Kommandointerface aus und öffnet selber
den JSON Kommandointerface Port. Somit werden effektiv alle neuen Verbindungen über den SessionProxy ablaufen.
Alle Verbindungen erfordern ab dann eine Authentifizierung gegenüber den auf dem Betriebssystem vorhandenen Benutzern.
Zugriffsrechte werden durch Gruppenmitgliedschaften des angegeben Benutzers geregelt und müssen in der SessionProxy
Konfiguration gesondert angegeben werden.
Ein Beispiel: Ein Nutzer wählt über die Android App eine Szene zur Ausführung aus.
Der SessionProxy wird diese Ausführungsanfrage nur an den SceneServer weiterleiten, wenn der Nutzer sich als "abc"
beim Start der Android App authorisiert hat.

Plugin und Plugin<->SceneServer Kommunikation:
----------------------------------------------
Plugins sind als eigene Prozesse modelliert, welche selbstständig eine Kommunikationsverbindung zum Server aufbauen müssen.
Da viele Qt Container und Basisklassen serialisiert werden können, wird ein einfaches QDataStream basiertes Protokoll für
die Kommunikation zwischen Plugin Prozess und SceneServer verwendet. Unter Windows werden NamedPipes, unter Linux/MacOS
UnixSockets verwendet.
Plugins können Eigenschaften (properties) besitzen und auf Eigenschaftsänderungen des Servers oder anderer Plugins
reagieren.
Ein Plugin muss ein C++/Qt PluginInterface implementieren. Entsprechend markierte Funktionen dienen als Ereignis, Bedingungs- oder
Aktionsmethoden und können vom SceneServer direkt genutzt werden.

Datenspeicherung:
-----------------
Ereignisse, Bedingungen, Aktionen und Szenen werden als JSON Objekte direkt auf dem Dateisystem hinterlegt, da
nach umfangreicher Evaluierung von dokumentbasierten Datenbanken diese keine weiteren Vorteile nach sich ziehen. Durch
Betriebssystemroutinen kann der SceneServer über Änderungen an den Dateien informiert werden. Übliche Dateiverwaltungs-
werkzeuge ermöglichen das manuelle Replizieren oder regelmäßige sichern der Daten und auf Dateisystemebene vorhandene
Konsistenzdaten sorgen für die nötige Datensicherheit auf Dateiebene.

Beispiel: Ereignisse, Bedingungen und Aktionen:
-----------------------------------------------
Ein Ereignis kann das Eintreten eines gewissen Zeitpunktes sein,
eine Bedingung könnte den aktuellen Steckdosenzustand meinen,
eine Aktion löst eine Veränderung aus etwa das Ändern der Lichwerte von Leuchtdioden.

Szenen:
-------
Ereignisse, Bedingungen und Aktionen machen erst Sinn, sobald diese zusammengefasst werden können.
Dies geschieht durch Szenen. Sobald ein, an eine Szene gebundenes, Ereignis auftritt,
werden die Bedingungen geprüft und dann ggfs. die in die Szene eingebundenen Aktionen,
evtl. mit eingestellter zeitlicher Verzögerung, ausgeführt.


Anwendungsgebiete:
==================
1. Intelligenter Wecker
2. Energiesparsystem
3. Intelligenter Raum
4. Automatisiertes Heimkino

Grafisches Programm zur Gestaltung von Profilen:
================================================
Um Ereignisse, Bedingungen und Aktionen in Szenen grafisch zu gestalten befindet
sich eine HTML5/JS Browser Anwendung im Ordner "editor".
Diese greift per Webdav Protokoll auf die JSON Ressourcen zu. Entsprechend müssen
die JSON Ressourcen in einem Ordner liegen, der sowohl vom SceneServer als auch
von einem Http Server mit WebDav Unterstützung ausgelesen und verändert werden kann.
Wenn der WebSocketProxy und optional der SessionProxy aktiv ist, können Aktionen
auf dem SceneServer über die Html Oberfläche ausgelöst werden. Außerdem wird das
Erstellen und Parametriesieren von Ereignissen und Bedingungen erleichert, da
mögliche Parameter direkt vom SceneServer abgefragt werden können.


Software bauen:
===============
Es wird CMake (www.cmake.org) benötigt. Im CMake-Gui Programm kann grafisch
der Quellcodeordner und ein davon unabhängiger BUILD-Ordner angegeben werden.
CMake versucht alle benötigten Abhängigkeiten und den angegebenen C++ Compiler
zu finden. Im nachfolgendem Abschnitt sind die Abhängigkeiten noch einmal
aufgeführt. Nach dem Generierungsschritt liegen (je nach Auswahl) Visual Studio
Projektdateien, Makefiles, usw vor.

Unter Windows: Hier wird VisualStudio 10+ empfohlen. Auf der Konsole kann nmake
verwendet werden, ansonsten lässt sich das Projekt auch mit der VS IDE bauen.

Unter Linux: Hier bietet sich das Erstellen lassen von Makefiles an. Anschließend
in den BUILD-Ordner wechseln und den Befehl "make" ausführen. Um ein Deb Packet
generieren zu lassen den Befehl "make package" nutzen. Fertige monolitische DEB
Pakete lassen sich im Downloadbereich herunterladen.

Work in Progress: 3. Quartal 2012:
* Websocketproxy
* Pluginverbesserungen

ToDo: 4. Quartal 2012:
* Dokumentation unter www.sceneserver.de.vu inkl. Architekturschaubildern :D
* HTML5 Editor: Evaluierung des JS Toolkits läuft
* SessionProxy
* Leds/Schalter Android App
* Beamer Android App
* Alarm Android App

Benötigte Bibliotheken für den Server ohne Plugins:
===================================================
Kernprozess:
------------
Name          Ubuntu Paket          Beschreibung
* Qt4           libqt4-dev            Qt4 Framework

SessionProxy:
-------------
Name          Ubuntu Paket          Beschreibung
* PAM           libpam0g-dev          Benutzerauthentifizierung

Plugins:
--------
Name          Ubuntu Paket          Beschreibung
* *PA:Pulseaudio -
* PA:GLib       -
* Linux_input_events:udev  -