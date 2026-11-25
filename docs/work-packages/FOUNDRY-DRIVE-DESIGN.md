# FoundryDrive -- Architektur- und Designgrundlage

**Status:** Beschlossene Architektur für die nächste
FoundryDrive-Implementierungsphase\
**Geltungsbereich:** Dateioperationen, blockweise IEC-Übertragung und
direkter Track-/Sector-Zugriff\
**Hinweis:** Dieses Dokument hält die gemeinsam getroffenen
Architekturentscheidungen fest. Es beschreibt das Zielbild, nicht den
bereits implementierten Zustand.

## 1. Ziele

FoundryDrive soll einen SD2IEC-artigen Datei- und DOS-Zugriff
ermöglichen, ohne eine vollständige 1541-Emulation oder die
Programmierung der 1541 über den IEC-Bus zum Ziel zu machen.

Die Architektur soll insbesondere:

-   PRG-Dateien während der tatsächlichen IEC-Übertragung sektorweise
    lesen, statt sie vorher vollständig einzulesen.
-   Mehrere gleichzeitig geöffnete logische Kanäle unabhängig
    voneinander unterstützen.
-   Die D64-spezifische Dateisystemlogik vollständig in `IECFoundryDisk`
    kapseln.
-   Direkte DOS-Sektorzugriffe auf vollständige Sektoren ermöglichen.
-   Schreibzugriffe architektonisch vorsehen, ohne sie in diesem Schritt
    vollständig umzusetzen.
-   Die GUI-Aktivitätsanzeige an tatsächliche Sektorzugriffe koppeln.

## 2. Verantwortlichkeiten

### 2.1 IECFoundryDisk

`IECFoundryDisk` ist für das DiskImage und dessen D64-spezifische
Dateisystem- und Sektorlogik verantwortlich.

Sie übernimmt:

-   Directory-Suche und Dateinamensauflösung.
-   Prüfung einer Datei-Sektorkette beim Öffnen.
-   Interpretation der D64-Sektorverknüpfung und der EOF-Kodierung.
-   Lesen und Schreiben vollständiger physischer Sektoren.
-   Umwandlung eines Dateisektors in einen Dateidatenblock
    einschließlich Nutzdatenlänge und Folgeinformation.

Die Disk hält **keinen dauerhaften Zustand pro geöffneter Datei**. Sie
speichert insbesondere weder den aktuellen Lesesektor noch eine Liste
besuchter Sektoren für eine Dateioperation. Alle für einen einzelnen
Zugriff benötigten Informationen werden über die Schnittstelle übergeben
oder im jeweiligen Aufruf verarbeitet.

### 2.2 IECFoundryDrive

`IECFoundryDrive` steuert die Dateioperationen und verwaltet deren
Laufzeitzustand.

Sie übernimmt:

-   Zuordnung einer Dateioperation zum jeweiligen logischen IEC-Kanal.
-   Speicherung des aktuellen Track/Sector pro aktivem Lesekanal.
-   Anforderung des nächsten Dateidatenblocks bei der Disk.
-   Steuerung des Ablaufs zwischen Dateioperation und IEC-Übertragung.
-   Verwaltung der eingehenden Daten und des Operationsfortschritts bei
    späteren Schreibzugriffen.

Das Drive interpretiert keine D64-Verknüpfungsbytes und keine
D64-EOF-Kodierung. Es verarbeitet ausschließlich die von der Disk
gelieferten Ergebnisinformationen.

### 2.3 IECProtocol

`IECProtocol` bleibt unabhängig vom DiskImage und vom D64-Dateisystem.

Es kennt:

-   Datenblöcke, die übertragen werden sollen.
-   Ob ein Datenblock der letzte Block einer Übertragung ist.
-   Den Übertragungsfortschritt und die IEC-Protokollzustände.

Es kennt ausdrücklich **keine** Dateinamen, Track-/Sector-Verknüpfungen,
BAM-Strukturen oder D64-EOF-Kodierung.

Bei einer blockweisen Dateiübertragung darf ein Zwischenblock nicht wie
eine vollständige IEC-Übertragung behandelt werden: EOI darf erst am
letzten Datenbyte der gesamten Dateiübertragung ausgelöst werden. Nach
einem vollständig übertragenen Zwischenblock muss der nächste Block
angefordert werden können.

## 3. Datei öffnen: atomisches OPEN

Das Öffnen einer Datei ist nach außen hin atomar.

Das Drive übergibt den Dateinamen einmalig an die Disk. Die Disk führt
intern die vollständige Prüfung durch:

1.  Directory-Eintrag suchen.
2.  Ersten Track/Sector ermitteln.
3.  Die gesamte Sektorkette durchlaufen.
4.  Zyklen erkennen.
5.  Ungültige Track-/Sector-Verweise und Sektor-Lesefehler erkennen.
6.  Die EOF-Kodierung des letzten Sektors auf Plausibilität prüfen.

Das Ergebnis des OPEN enthält:

-   Erfolg oder Fehler.
-   Bei Erfolg den ersten Track/Sector, sofern die Datei Daten enthält.
-   Eine eindeutige Kennzeichnung für eine leere Datei.
-   Bei Fehlern einen geeigneten Fehlerstatus.

Erst nach erfolgreicher Prüfung gilt OPEN als erfolgreich. Bei einem
Fehler werden keine Dateidaten an den C64 übertragen.

Die Prüfung beim OPEN ist eine gezielte Laufzeitprüfung der geöffneten
Datei. Sie ersetzt **keine vollständige Image-Validierung**. Eine
spätere Validierung kann beispielsweise zusätzlich verwaiste Sektoren,
Überschneidungen zwischen Dateien und Widersprüche in der BAM
untersuchen.

### 3.1 Keine gespeicherte Sektorkette erforderlich

Die Disk muss die beim OPEN durchlaufene Sektorkette nicht speichern.
Nach erfolgreichem OPEN genügt dem Drive der erste Track/Sector. Das
Drive führt den aktuellen Track/Sector für jeden logischen Kanal selbst.

Die Disk muss deshalb beim anschließenden Lesen keine Dateioperation
oder Dateikette anhand eines internen Handles wiederfinden.

## 4. Datei blockweise lesen

Nach erfolgreichem OPEN fordert das Drive die Dateidaten sektorweise an.

Für jeden Zugriff übergibt das Drive den aktuellen Track/Sector an die
Disk. Die Disk:

1.  Liest den angeforderten Sektor.
2.  Interpretiert intern die D64-Verknüpfungsbytes.
3.  Entfernt die beiden Verknüpfungsbytes aus den Dateidaten.
4.  Bestimmt die tatsächlich gültige Nutzdatenlänge.
5.  Liefert die Nutzdaten sowie die Information über das Dateiende
    zurück.

Das Ergebnis eines Dateiblock-Lesezugriffs enthält logisch:

-   Dateinutzdaten.
-   Tatsächliche Nutzdatenlänge.
-   `isLast`.
-   Bei einem Zwischenblock den nächsten Track/Sector.
-   Einen Erfolgs- oder Fehlerstatus.

Beim letzten Sektor kann die Nutzdatenlänge kleiner als 254 Bytes sein.
Die beiden Verknüpfungsbytes gehören nie zu den Dateinutzdaten. Die Disk
wertet die D64-EOF-Kodierung aus und liefert dem Drive bereits die
korrekte Nutzdatenlänge.

Das Drive speichert bei einem Zwischenblock den von der Disk gelieferten
nächsten Track/Sector und fordert diesen erst an, wenn der vorherige
Datenblock vollständig übertragen wurde.

Die Disk speichert zwischen zwei Aufrufen keinen Lesestand.

## 5. Mehrere gleichzeitig geöffnete Dateien

Ein IEC-Gerät kann mehrere logische Kanäle gleichzeitig bedienen. Jede
Dateioperation muss deshalb unabhängig von den anderen Kanälen
funktionieren.

Das Drive verwaltet pro logischem Kanal mindestens:

-   Ob eine Datei geöffnet ist.
-   Den zugehörigen Datei-/Operationsstatus.
-   Den aktuellen Track/Sector für den nächsten Lesezugriff.

Die Disk besitzt keinen globalen Dateizeiger und keinen globalen
Sektorkettenzustand. Zwei Kanäle können daher unabhängig voneinander
unterschiedliche Dateien lesen. Auch wenn zwei Dateien denselben
physischen Sektor referenzieren, beeinflussen sich ihre Leseoperationen
nicht gegenseitig.

Die beim OPEN erfolgte Kettenprüfung gehört jeweils zur gerade
geöffneten Datei und ist abgeschlossen, bevor Daten übertragen werden.

## 6. Direkter Track-/Sector-Zugriff

Zusätzlich zum dateibasierten Zugriff gibt es einen direkten Zugriff auf
vollständige physische Sektoren. Dieser wird für DOS-Funktionen wie
`U1`/`U2` beziehungsweise `B-R`/`B-W` benötigt.

Dafür stehen konzeptionell getrennte Operationen zur Verfügung:

-   `readSector(track, sector)`: vollständigen Sektor mit allen 256
    Bytes lesen.
-   `writeSector(track, sector, data)`: vollständigen Sektor mit allen
    256 Bytes schreiben.

Bei diesen Operationen:

-   findet keine Directory-Suche statt;
-   wird keine Datei-Sektorkette geprüft oder verfolgt;
-   werden keine Verknüpfungsbytes entfernt;
-   wird keine Dateinutzdatenlänge abgeleitet.

Der direkte Sektorzugriff arbeitet exakt mit dem angeforderten
physischen Sektor. Die Disk bleibt dabei alleinige Stelle für die
Prüfung der Track-/Sector-Gültigkeit und den Zugriff auf das DiskImage.

Dateibasierter Zugriff und direkter Sektorzugriff sind zwei
unterschiedliche Zugriffsebenen auf dieselbe Disk-Implementierung.

## 7. Schreibzugriffe: Erweiterbarkeit

Schreibzugriffe werden in der Architektur ausdrücklich berücksichtigt,
auch wenn sie nicht zwingend Bestandteil der ersten
Implementierungsstufe sind.

Die Zuständigkeiten sollen spiegelbildlich zum Lesen bleiben:

-   Das Drive verwaltet den Schreibvorgang pro logischem Kanal und nimmt
    die vom IEC-Protokoll empfangenen Daten entgegen.
-   Das IECProtocol liefert empfangene Daten beziehungsweise das Ende
    der Übertragung, ohne D64-Strukturen zu kennen.
-   Die Disk übernimmt die D64-spezifische Ablage der Dateidaten in
    Sektoren, Sektorzuweisung und -verknüpfung sowie die erforderlichen
    Directory- und BAM-Aktualisierungen.

Die Schnittstelle darf daher nicht voraussetzen, dass Dateioperationen
ausschließlich lesend sind. Die konkrete Semantik für parallele
Schreibzugriffe, Abbruch, Überschreiben und Fehlerbehandlung wird vor
der Implementierung des Schreibpfads gesondert festgelegt.

Der direkte `writeSector`-Zugriff bleibt davon getrennt und schreibt
einen vollständigen physischen Sektor.

## 8. IEC-Übertragung und EOI

Dateidaten werden blockweise übertragen. Die Disk liefert jeweils einen
Dateidatenblock; das Drive koordiniert dessen Übertragung mit dem
IECProtocol.

Wesentliche Regel:

-   Ein Zwischenblock ist **nicht** das Ende der IEC-Übertragung.
-   Der nächste Block wird erst angefordert, wenn der aktuelle Block
    vollständig übertragen wurde.
-   Nur beim letzten Dateiblock wird EOI am letzten gültigen Datenbyte
    ausgelöst.
-   Die tatsächliche Nutzdatenlänge des letzten Blocks ist maßgeblich;
    es dürfen keine ungenutzten Bytes des physischen Sektors übertragen
    werden.

Der bestehende `SendData`-Pfad kann für bereits vollständig vorbereitete
Antworten wie Directory-Daten und einfache DOS-Antworten erhalten
bleiben. Die blockweise Dateiübertragung benötigt zusätzlich eine
Möglichkeit, nach einem Zwischenblock den nächsten Block anzufordern,
ohne die IEC-Übertragung zu beenden.

## 9. GUI-Sektoraktivität

Die GUI-Aktivitätsanzeige soll tatsächliche Sektorzugriffe
widerspiegeln.

Daraus folgt:

-   Die interne Sektorkettenprüfung während OPEN erzeugt keine normalen
    GUI-Aktivitätsereignisse.
-   Beim tatsächlichen Lesen eines Dateisektors während der Übertragung
    entsteht das Sektorereignis beim realen Zugriff.
-   Direkte Sektorzugriffe erzeugen ebenfalls Ereignisse beim
    tatsächlichen Zugriff.
-   Die GUI soll keine künstliche Übertragungsgeschwindigkeit oder
    zeitlich simulierte Sektorfolge verwenden, um die Aktivität
    nachzubilden.

Die bestehende Thread-/Event-Übergabe zur GUI bleibt zu berücksichtigen.
Die Anzeige darf den IEC-Übertragungsvorgang nicht beeinflussen oder
ausbremsen.

## 10. Bewusste Abgrenzungen

Folgende Architekturregeln gelten:

-   Keine vollständige PRG-Datei vor dem IEC-Transfer vorladen.
-   Keine dauerhaften Dateioperations-Kontexte oder Dateizeiger in
    `IECFoundryDisk`.
-   Keine D64-spezifische Interpretation in `IECFoundryDrive` oder
    `IECProtocol`.
-   Keine D64-Dateisystemkenntnis im IECProtocol.
-   Keine Vermischung von Dateidatenzugriff und direktem vollständigem
    Sektorzugriff.
-   Keine EOI-Behandlung am Ende jedes einzelnen Zwischenblocks.
-   Keine GUI-Aktivitätsereignisse allein aufgrund der internen
    OPEN-Prüfung.
-   Keine vollständige 1541-Emulation als Voraussetzung für die hier
    beschriebenen Funktionen.

## 11. Vorgesehene Implementierungsreihenfolge

Die konkrete Reihenfolge kann bei der Umsetzung an den vorhandenen Code
angepasst werden. Inhaltlich bietet sich an:

1.  Ergebnis- und Datenstrukturen für atomisches Datei-OPEN und
    Dateiblock-Lesen festlegen.
2.  D64-Sektorkettenprüfung beim OPEN in `IECFoundryDisk`
    implementieren.
3.  Zustandslosen Dateiblock-Lesezugriff in `IECFoundryDisk`
    implementieren.
4.  Datei-/Lesestand pro logischem Kanal in `IECFoundryDrive` verwalten.
5.  IECProtocol um die blockweise Fortsetzung ohne vorzeitiges EOI
    ergänzen.
6.  Tatsächliche Sektorereignisse mit dem realen Lesezeitpunkt verbinden
    und GUI-Verhalten prüfen.
7.  Direkte vollständige Sektorzugriffe für DOS-Lese-/Schreibbefehle
    ergänzen beziehungsweise überprüfen.
8.  Schreibzugriffe auf Dateiebene in einem separaten Umsetzungsschritt
    spezifizieren und implementieren.

## 12. Abnahmekriterien für den dateibasierten Leseweg

-   Ein OPEN prüft die gesamte Sektorkette, bevor es Erfolg meldet.
-   Ein Zyklus oder ein ungültiger Sektorverweis wird beim OPEN als
    Fehler gemeldet; es beginnt keine teilweise Dateiübertragung.
-   Eine leere Datei wird korrekt erkannt.
-   Jeder logische Kanal besitzt seinen eigenen Lesestand.
-   Ein Dateisektor wird erst bei Anforderung für die tatsächliche
    Übertragung gelesen.
-   Die Disk liefert ausschließlich gültige Dateinutzdaten und deren
    tatsächliche Länge.
-   Der letzte Block wird mit der korrekten Länge übertragen.
-   EOI wird ausschließlich am letzten gültigen Byte der gesamten
    Dateiübertragung ausgelöst.
-   Die GUI-Sektoraktivität folgt den tatsächlichen Lesezugriffen und
    nicht dem OPEN-Prüfdurchlauf.
-   Direkte Sektorzugriffe liefern beziehungsweise schreiben
    vollständige 256-Byte-Sektoren unabhängig von Dateioperationen.
