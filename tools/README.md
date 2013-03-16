=Inhalt=
In diesem Ordner befinden sich Programme, die nicht für den Einsatz des Servers erforderlich sind.

===linux_ircore_xorg===
Verhindert das Tastenereignisse der ATI Remote an den X-Server weitergeleitet werden, da die Tastenereignisse
ja bereits von SceneControl ausgewertet werden. Analog können Dateien für weitere Linux Eingabegeräte erstellt
werden.

===modify_registry_path===
Für den Windows NSIS Installer. Das Programm wird gebaut und im Installer unter Windows genutzt um den
Installationspfad in die Registry zu schreiben. Der Server liest diesen Wert aus, um etwa den Installationsort
der Plugins finden zu können.

===startonboot===
Für verschiedene Betriebssysteme enthaltene Programme um den Server beim Systemstart mit zu starten.

===installservicehelper===
Fügt in die verschiedenen json Dateien eines Services vor der Installation die fehlenden Schlüsselwertpaare
für componentid_ ein.