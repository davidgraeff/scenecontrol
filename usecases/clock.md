---
layout: documentation
title: Datenhaltung
tagline: Speicherung als JSON Dateien
group: documentation
comments: true
category : documentation
tags : [plugin]
---
{% include JB/setup %}
Szenen, Szenenelemente, aber auch Plugin Konfigurationen werden als JSON Objekte direkt auf dem Dateisystem hinterlegt. Die
Datenspeicherung ist durch eine eigene Bibliothek gekapselt um einen leichten Austausch durch andere Technologien zu ermöglichen.

Vorteile: Die Speicherung auf dem Dateisystem bietet das Erfassen von Änderungen der Daten durch
Drittanwendungen sowie die Nutzung üblicher Dateiverwaltungswerkzeuge. Manuelles Replizieren sowie
regelmäßige Sicherungen können somit auf Dateiebene genutzt werden (Dropbox, github, ...).
