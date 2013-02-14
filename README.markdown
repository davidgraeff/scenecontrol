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
Ereignisse, Bedingungen, Aktionen und Szenen werden als JSON Objekte direkt auf dem Dateisystem hinterlegt. Die
Datenspeicherung ist jedoch durch eine eigene Bibliothek gekapselt um weitere Datenspeicherungssysteme leichter
nutzbar zu machen. Die Speicherung auf dem Dateisystem bietet jedoch bereits das Erfassen von Änderungen der Daten durch
Drittanwendungen sowie die Nutzung üblicher Dateiverwaltungswerkzeuge. Manuelles Replizieren sowie
regelmäßige Sicherungen können auf Dateiebene genutzt werden (Dropbox, github, ...).

Szenen:
-------
Eine Szene definiert sich durch einen Namen, gfs. zugewiesene Kategorien und Szenenelementen.
Szenenelemente sind Ereignisse, Bedingungen und Aktionen, welche als gerichteter Graph organisiert sind.
Ein Graph enthält Knoten und Kanten, welche die Knoten verbinden. Übertragen auf eine Szene
sind dadurch i.d.R. Ereignisse über Kanten mit Bedingungen verknüpft, welche wiederum mit
Aktionen verbunden sind. Eine solche Kette von Szenenelementen heißt hier Ereigniskette.
Eine Szene kann mehrere Ereignisketten enthalten.

Ein Beispiel: Ein Ereignis kann das Eintreten eines gewissen Zeitpunktes sein,
eine daran angebundene Bedingung könnte einen Steckdosenzustand prüfen,
eine darauf folgende Aktion löst etwa das Ändern der Lichwerte von Leuchtdioden aus.

Anwendungsgebiete:
==================
1. Intelligenter Wecker
2. Energiesparsystem
3. Intelligenter Raum
4. Automatisiertes Heimkino

Grafisches Programm zur Verwaltung von Szenen:
==============================================
Um Ereignisse, Bedingungen und Aktionen in Szenen grafisch zu gestalten befindet
sich eine HTML5/JS Browser Anwendung im Ordner "htmleditor".
Der SceneServer und der WebSocketProxy müssen laufen. Außerdem wird das
Erstellen und Parametriesieren von Ereignissen und Bedingungen erleichert, da
mögliche Parameter direkt vom SceneServer abgefragt werden können.

Weiterentwicklung:
==================
Work in Progress: 1. Quartal 2013:
* Plugin: dmx/artnet
* Fetch/Install scripts

ToDo: 2. Quartal 2013:
* Dokumentation inkl. Architekturschaubildern
* SessionProxy
* Android App: Modulare Plugin Unterstützung statt monolitischer Block?, Bugfixes

Fertige Binaries:
=================
* Ubuntu 12.04, 12.10, 13.04 (x86, x64, ARM_Hf): https://launchpad.net/~david-graeff/+archive/scenecontrol

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
generieren zu lassen den Befehl "make package" nutzen.

Abhängigkeiten:
---------------
| Name        | Ubuntu Paket           | Beschreibung  | Komponenten
| ------------- |:-------------:| -----:| -----:|
| Qt4      | libqt4-dev | Qt4 Framework | Kernprozess
| SSL      | libssl-dev | Sichere Verbindung | Kernprozess, Kontrollsocket
| PAM      | libpam0g-dev      |   Benutzerauthentifizierung | SessionProxy
| Pulseaudio | libpulse-dev      |    Pulseaudio | Plugin: Pulseaudio
| UDEV | libudev-dev      |    Linux Input Events | Plugin: Linux_input_events
