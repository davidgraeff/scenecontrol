---
layout: documentation
title: Ladevorgang und Ausführung
tagline: Vorgänge beim Laden und während der Ausführung
group: documentation
comments: true
category : documentation
tags : [loading]
---
{% include JB/setup %}
### Ladevorgang
Der SceneServer baut nach dem Start eine Verbindung zur _Datenhaltung_ auf und fordert Plugin Konfigurationen an.

Aus der _Datenhaltung_ werden anschließend alle Szenen angefordert und in ausführbare Objekte überführt.
Alle auf diesen Szenen registrierten Ereignisse werden bei den zuständigen Plugin-Instanzen registriert.

### Ausführung
Sobald ein registriertes Ereignis eintritt, wird der SceneServer kontaktiert und die mit dem Ereignis verbundene
Szene wird <span class="tooltip" title="Was mit Ausführung einer Szene gemeint ist, wird weiter unten erläutert.">ausgeführt</span>.

