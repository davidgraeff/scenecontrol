---
layout: documentation
title: Dokumentation
tagline: SceneControl Architektur
group: navigation
comments: true
weight : 9
category : documentation
tags : [command socket, proxy, websocket]
---
{% include JB/setup %}
<img src="../images/Architektur2.png" style="" />
Der Kernprozess, im Schaubild mittig dargestellt, wird im weiteren SceneServer genannt. Dieser Kernprozess
ist sehr minimalistisch konstruiert um die Fehleranfälligkeit zu minimieren. Die Datenhaltung läuft in diesem Prozesskontext,
der <span class="tooltip" title="TCP-Socket, welcher JSON-Nachrichten annimmt">Kommando-Socket</span> und ein Thread-Pool.
Die Ausführung von Szenen geschieht jeweils in eigenen Threads. Die eigentliche Funktionalität wird über Pluginprozesse bereitgestellt. In der Grafik sind diese
um den Kernprozess herum skizziert. Pluginprozesse können untereinander kommunizieren (Abhängigkeitsverhältnis).

Durch die Trennung in Prozesse mit fest gelegtem, eingegrenztem Aufgabenbereich ist die Ausfallsicherheit der
gesamten Architektur besonders hoch und für den 24/7 Betrieb ausgelegt.

### Interprozess-Kommunikation
Plugins sind als eigene Prozesse modelliert, welche selbstständig eine Kommunikationsverbindung zum Server aufbauen müssen.
Da viele Qt Container und Basisklassen serialisiert werden können, wird ein einfaches QDataStream basiertes Protokoll für
die Kommunikation zwischen Plugin Prozess und SceneServer verwendet. Unter Windows werden NamedPipes, unter Linux/MacOS
UnixSockets verwendet.

Plugins können Eigenschaften (_properties_) besitzen und auf Eigenschaftsänderungen des Servers oder anderer Plugins
reagieren. Für C++ Plugins gibt es ein PluginInterface, welches zu implementieren ist. Theoretisch können auch andere
Programmiersprachen für Plugins verwendet werden.

### Szenen
Eine Szene definiert sich durch einen Namen, gfs. zugewiesene Kategorien und Szenenelementen.
Szenenelemente sind Ereignisse, Bedingungen und Aktionen, welche als gerichteter Graph organisiert sind.
Ein Graph enthält Knoten und Kanten, welche die Knoten verbinden. Übertragen auf eine Szene
sind dadurch i.d.R. Ereignisse über Kanten mit Bedingungen verknüpft, welche wiederum mit
Aktionen verbunden sind. Eine solche Kette von Szenenelementen heißt hier _Ereigniskette_.

Eine Ereigniskette kann Verzweigungen besitzen, also von einem Element kann diese zu zwei oder mehr Elementen
weiterführen. Eine Szene kann mehrere Ereignisketten enthalten. Weiter oben wurde festgestellt,
dass eine Szene bei Eintritt eines seiner Ereignisse ausgeführt wird. Tatsächlich wird aber
nur die Ereigniskette ab dem Punkt abgearbeitet, wo sich dieses Ereignis befindet.

__Ein Beispiel__: Ein Ereignis kann das Eintreten eines gewissen Zeitpunktes sein,
eine daran angebundene Bedingung könnte einen Steckdosenzustand prüfen,
eine darauf folgende Aktion löst etwa das Ändern der Lichwerte von Leuchtdioden aus.
