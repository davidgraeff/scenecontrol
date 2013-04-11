Heimautomatisierung Framework
=============================
Eine einfach gehaltenes Kernprogramm führt Szenenabläufe aus. In einer Szene sind
frei verknüpft Ereignisse, Bedingungen und Aktionen angeordnet. Funktionalität wird durch
lokale oder entfernte "__Service__" Prozesse bereichgestellt.

Alle weiteren Informationen, die Roadmap
und Downloads befinden sich auf der [Webseite](http://davidgraeff.github.com/scenecontrol)  dieses Projekts.

Anwendungsgebiete
=================
* Intelligenter Wecker
* Energiesparsystem
* Intelligenter Raum
* Automatisiertes Heimkino

Entwicklungsdetails
===================
Das Kernprogramm ist in JavaScript erstellt und läuft als __node js__ Programm. Durch die
Verwendung von JavaScript können sich der Editor und das Kernprogramm Quellcode teilen.
Der Datenaustausch erfolgt über JSON. Szenen, Szenenelemente und alle weiteren Daten werden
in einer mongoDB Datenbank gehalten.

Scenes Editor
=============
Um Ereignisse, Bedingungen und Aktionen zu erstellen und grafisch in Szenen zu organisieren
exitiert eine Web-Anwendung. Die Anwendung kann direkt durch das Aufrufen der _index_
Datei gestartet oder über einen beliebigen Webserver ausgeliefert werden. Eine Demonstartion
befindet sich auf der Webseite des Projekts.

Screenshot:
![Alt text](http://davidgraeff.github.com/scenecontrol/images/editor-feb-2013.jpg)

Android-App
===========
Fast alle Funktionen der __Services__ können über eine Android App kontrolliert und ausgelöst werden.
Eine Funktionen des Editors sind auch in der App möglich, wie das Erstellen und Verknüpfen von Startzeiten mit Szenen.
![Alt text](http://davidgraeff.github.com/scenecontrol/images/androidapp.jpg)

Code Stabilität
===============
Über das Travis CI wird die ständige Kompilierbarkeit sichergestellt und automatisierte Tests
decken bereits einige Bereiche des Kernprogramms ab.
[![Build Status](https://travis-ci.org/davidgraeff/scenecontrol.png?branch=master)](https://travis-ci.org/davidgraeff/scenecontrol)
Auf der Webseite befindet sich eine Roadmap. Gemeldete Fehler und Wünsche werden über das Ticketsystem von github verwaltet.

Installation
============
Es wird node js, cmake und ein c++ compiler benötigt.

* BUILD Verzeichnis erstellen, z.B. "./__build__"
* cmake im BUILD Verzeichnis ausführen, z.B. "cmake ../"
* Installieren mit "make install"
* Ausführen des Servers mit "__sceneserversession__"

Die Ausführung erfolgt dabei in einem screen Fenster. Der Server kann auch direkt gestartet werden,
etwa mit "nodejs /usr/lib/scenecontrol_suite/core/main.js".