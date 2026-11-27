# WorkPackage: IEC-Blockzugriffe U1/U2

## 1. Ziel

Implementierung der direkten Sektorzugriffe über den IEC-Bus entsprechend den Commodore-DOS-Befehlen `U1` und `U2`.

Die Befehle ermöglichen es einem C64-Programm, einen beliebigen Sektor zu lesen oder zu schreiben, ohne den normalen Datei-Zugriff über LOAD oder SAVE zu verwenden.

`U1` und `U2` sind die gebräuchlichen Kurzformen von `B-R` und `B-W`.

## 2. Umfang

### 2.1 U1 – Block Read

Der C64 fordert einen bestimmten Track und Sektor an.

- Das DOS-Kommando wird über den Kommandokanal 15 empfangen.
- Der angegebene Datenkanal wird als Blockpuffer verwendet.
- Die Disk liest den angeforderten Sektor.
- Der vollständige Sektor mit 256 Byte wird über den angegebenen IEC-Kanal an den C64 übertragen.
- Ungültige Track-/Sektor-Angaben und Lesefehler werden über den DOS-Statuskanal gemeldet.

### 2.2 U2 – Block Write

Der C64 überträgt einen vollständigen Sektor an die Disk.

- Das DOS-Kommando wird über den Kommandokanal 15 empfangen.
- Der angegebene Datenkanal wird als Blockpuffer verwendet.
- Die Disk empfängt die 256 Byte über IEC.
- Nach vollständigem Empfang wird der Sektor geschrieben.
- Ungültige Track-/Sektor-Angaben und Schreibfehler werden über den DOS-Statuskanal gemeldet.

### 2.3 DOS-Kommandos

Unterstützt werden:

- `U1: Kanal, Laufwerk, Track, Sektor`
- `U2: Kanal, Laufwerk, Track, Sektor`
- `B-R: Kanal, Laufwerk, Track, Sektor`
- `B-W: Kanal, Laufwerk, Track, Sektor`

`U1` und `B-R` müssen dasselbe Verhalten haben. Entsprechend müssen `U2` und `B-W` identisch arbeiten.

Der Laufwerksparameter wird zunächst entsprechend der üblichen 1541-Semantik behandelt. Die konkrete Zuordnung und Validierung wird bei der Implementierung festgelegt.

## 3. Architektur

### IECFoundryDrive

Das Drive übernimmt:

- Empfang und Auswertung der DOS-Kommandos.
- Zuordnung des angegebenen Kanals.
- Verwaltung der geöffneten Blockkanäle.
- Steuerung der IEC-Datenübertragung.
- Weitergabe der Sektoroperationen an die Disk.
- Setzen des DOS-Status bei Fehlern.

Die Kanalzustände müssen unterscheiden können zwischen:

- Datei-Kanal
- Block-Lese-Kanal
- Block-Schreib-Kanal
- DOS-Kommandokanal

Die bisherige pauschale Behandlung empfangener Daten als Dateiname darf dadurch nicht weitergeführt werden.

### IECFoundryDisk

Die Disk bleibt für die eigentlichen Sektoroperationen verantwortlich:

- Sektor lesen
- Sektor schreiben
- Track-/Sektor-Validierung
- Image-Zustand und Schreibänderungen verwalten
- Sektorzugriffe für die GUI protokollieren

Die Disk kennt weiterhin keine IEC-Kanalnummern und keine IEC-Protokollzustände.

### IECProtocol

Das IEC-Protokoll bleibt unabhängig von den DOS-Befehlen.

Die bestehenden Ereignisse und Datenübertragungen werden verwendet. Es ist zunächst keine Änderung an der grundsätzlichen Bus-/Protokollarchitektur vorgesehen.

## 4. Fehlerbehandlung

Mindestens folgende Situationen müssen korrekt behandelt werden:

- Kein gültiges Disk-Image: `74, DRIVE NOT READY`
- Ungültiger Track oder Sektor: `66, ILLEGAL TRACK OR SECTOR`
- Lesefehler: `21, READ ERROR`
- Schreibfehler: `25, WRITE ERROR`
- Ungültiger oder nicht verfügbarer Kanal
- Unvollständige Blockübertragung
- Falsche Datenlänge

Ein fehlgeschlagener Blockzugriff darf nicht zu einem hängenden IEC-Zustand führen.

## 5. Abgrenzung

Nicht Bestandteil dieses WorkPackages:

- Normales SAVE von PRG-Dateien
- BAM-Verwaltung und Sektorreservierung für Dateien
- Directory-Einträge anlegen oder ändern
- SCRATCH, RENAME, COPY, NEW
- INITIALIZE und VALIDATE
- Vollständige 1541-Emulation oder 1541-Maschinencode-Kompatibilität

Diese Funktionen werden in späteren WorkPackages behandelt.

## 6. Umsetzungsschritte

1. Bestehende Kanalverwaltung analysieren und um Kanaltypen erweitern.
2. DOS-Kommandoempfang und Parser für U1/U2 sowie B-R/B-W implementieren.
3. U1 implementieren: Sektor lesen und über den angegebenen Kanal übertragen.
4. U2 implementieren: 256 Byte empfangen und Sektor schreiben.
5. Fehlerfälle und DOS-Statusmeldungen vervollständigen.
6. IEC-Übertragung und GUI-Sektoraktivität testen.
7. Verhalten mit realen C64-BASIC-Programmen überprüfen.

## 7. Abnahmekriterien

- U1 liest einen beliebigen gültigen Sektor vollständig.
- U2 schreibt einen vollständigen Sektor korrekt.
- B-R und B-W funktionieren als gleichwertige Alternativen.
- Mehrere geöffnete Kanäle beeinflussen sich nicht gegenseitig.
- Ein Blockzugriff funktioniert auch nach vorherigem LOAD.
- Fehler werden über Kanal 15 gemeldet.
- Fehlerhafte oder unvollständige Übertragungen lassen das IEC-Protokoll nicht hängen.
- Sektorzugriffe werden in der GUI korrekt angezeigt.
- Das Disk-Image enthält nach U2 die tatsächlich geschriebenen Daten.

