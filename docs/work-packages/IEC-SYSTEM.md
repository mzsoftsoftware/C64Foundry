# Work Package: IECSystem

**Status:** Entwurf zur Abstimmung  
**Voraussetzung:** IEC-Architektur-Refactoring  
**Ziel:** Zentrale IEC-Laufzeitkomponente einführen

## 1. Ziel

`IECSystem` bildet die zentrale Laufzeitkomponente des IEC-Subsystems.

Es verbindet Bus, Protokoll, Geräteverwaltung und Geräteerzeugung. Es koordiniert den Lebenszyklus der IEC-Geräte und stellt dem `EmulatorWorker` eine einheitliche Schnittstelle zur Verfügung.

Dabei gelten folgende Grundsätze:

- Der `EmulatorWorker` kennt keine konkreten IEC-Geräte.
- `IECSystem` koordiniert die IEC-Komponenten.
- `IECDeviceManager` übernimmt ausschließlich Registrierung und Kommunikation-Routing.
- `IECDeviceFactory` erzeugt und besitzt konkrete IEC-Geräte und deren Backend-Ressourcen.
- Das Drive verwaltet selbst seine Disk-Adressen und Disk-Zustände.
- Im Emulationszyklus gibt es genau einen zentralen IEC-Taktpfad.

## 2. Zielarchitektur und Ownership

```text
EmulatorWorker
 |
 |-- C64Machine
 |
 |-- IECDeviceFactory
 |    |
 |    |-- IECAsciiPrinter
 |    |
 |    |-- IECFoundryDrive
 |         |
 |         |-- Disk-Zustand #8
 |         |-- Disk-Zustand #9
 |         |-- Disk-Zustand #10
 |         `-- ...
 |
 |-- IECSystem
 |    |
 |    |-- IECBus
 |    |-- IECProtocol
 |    `-- IECDeviceManager
 |
 `-- EmulationScheduler
```

Der `EmulatorWorker` besitzt die Laufzeitkomponenten. Die `IECDeviceFactory` besitzt die von ihr erzeugten konkreten Geräte und deren gemeinsam genutzte Backend-Ressourcen.

`IECSystem` besitzt ausschließlich die zentralen IEC-Infrastrukturkomponenten:

- `IECBus`
- `IECProtocol`
- `IECDeviceManager`

`IECSystem` besitzt weder ein weiteres `IECSystem` noch direkt ein `IECFoundryDrive`.

Die Geräte werden durch die Factory erzeugt. `IECSystem` koordiniert ihre Registrierung beim Manager und ihre Entfernung vor der Zerstörung.

## 3. Verantwortlichkeiten

### 3.1 IECSystem

`IECSystem` ist die zentrale Fassade für alle IEC-Funktionen.

Aufgaben:

- Erzeugen und Zerstören von `IECBus`, `IECProtocol` und `IECDeviceManager`.
- Verbindung des IEC-Busses mit der `C64Machine`.
- Koordination der Kommunikation zwischen Protokoll und Geräte-Manager.
- Verwaltung des Geräte-Lebenszyklus in Zusammenarbeit mit der Factory.
- Weitergabe von Reset-Aufrufen an Protokoll und Geräte.
- Bereitstellung der IEC-Konfigurationsfunktionen für den Worker.
- Sammeln der gerätebezogenen Ausgabeevents.
- Bereitstellung der Drive-Aktivitätsereignisse als neutrale Event-Typen.
- Sicherstellen, dass Geräte vor ihrer Zerstörung aus dem Manager ausgetragen werden.

`IECSystem` enthält keine gerätespezifische Kommunikationslogik.

### 3.2 IECBus

Der Bus bleibt für die elektrischen IEC-Signale zuständig.

Er kennt weder Protokollbefehle noch Geräte.

### 3.3 IECProtocol

Es gibt genau eine zentrale Protokollinstanz.

Sie:

- liest und interpretiert die elektrischen Bus-Signale,
- erkennt IEC-Adressierung und Kommunikationsmodus,
- erzeugt neutrale Protokollereignisse,
- verarbeitet Antworten beziehungsweise Sendeaufträge des Managers,
- steuert die Bus-Signale entsprechend dem Protokollablauf.

Sie kennt keine konkreten Geräte und enthält keine Gerätebefehle.

### 3.4 IECDeviceManager

Der Manager übernimmt ausschließlich die Zuordnung und Kommunikation.

Er:

- registriert Geräte unter ihren IEC-Adressen,
- verhindert doppelte Adressregistrierungen,
- ordnet Protokollereignisse dem passenden Gerät zu,
- leitet Geräteantworten an das Protokoll zurück,
- entfernt Registrierungen vor der Zerstörung eines Gerätes.

Er erzeugt, besitzt und zerstört keine Geräte.

Er kennt keine konkreten Gerätetypen und verwaltet keine Disk-Images.

### 3.5 IECDeviceFactory

Die Factory ist für die konkreten Geräte zuständig.

Sie:

- erzeugt konfigurierte Geräte,
- besitzt die erzeugten Geräte,
- verwaltet gemeinsam benötigte Backend-Ressourcen,
- zerstört Geräte nach Aufforderung durch `IECSystem`.

Das `IECFoundryDrive` wird hier erzeugt und verwaltet. Die erzeugten Disk-Geräte verwenden dieses gemeinsame Drive.

Die Factory übernimmt keine Protokollkommunikation und kein Routing.

## 4. Geräte-Lebenszyklus

Das Anlegen eines Gerätes erfolgt über `IECSystem`:

1. `IECSystem` erhält eine Geräte-Konfiguration.
2. `IECSystem` fordert die Factory zur Erzeugung des Gerätes auf.
3. Die Factory erzeugt und übernimmt das Gerät.
4. `IECSystem` ermittelt dessen IEC-Adressen über die generische Geräte-Schnittstelle.
5. `IECSystem` registriert das Gerät mit seinen Adressen beim Manager.
6. Bei einem Fehler wird das Gerät wieder freigegeben und die Operation schlägt fehl.

Beim Entfernen:

1. `IECSystem` lässt das Gerät beim Manager austragen.
2. Erst danach fordert `IECSystem` die Factory zur Zerstörung auf.
3. Die Factory gibt das Gerät frei.

Damit kann der Manager niemals einen Zeiger auf ein bereits zerstörtes Gerät behalten.

## 5. Taktung und Emulationszyklus

Der bisherige Scheduler taktet jedes IEC-Gerät einzeln. Das entfällt mit dem zentralen Protokoll.

Der neue Ablauf pro C64-Zyklus lautet:

```text
EmulationScheduler::clock()
 |
 |-- C64Machine::clock()
 |
 `-- IECSystem::clock()
      |
      |-- IECProtocol verarbeitet Bus-Signale
      |
      |-- Protokollereignisse an IECDeviceManager
      |
      |-- Manager dispatcht an das adressierte Gerät
      |
      `-- Geräteantworten zurück an IECProtocol
```

Die genaue Reihenfolge innerhalb von `IECSystem::clock()` muss beim Protokollumbau festgelegt werden. Entscheidend ist, dass Protokoll und Geräte nicht zusätzlich an anderer Stelle getaktet werden.

Es gibt keine Factory-Aufrufe, Registrierungen, Speicherallokationen oder Geräte-Lebenszyklusänderungen im normalen Taktpfad.

## 6. Schnittstelle von IECSystem

Die öffentliche Schnittstelle soll dem Worker alle bisher benötigten IEC-Funktionen anbieten, ohne konkrete Geräteklassen offenzulegen.

Vorgesehene Funktionsgruppen:

### Laufzeit

- Konstruktor mit den benötigten Laufzeitabhängigkeiten.
- Destruktor.
- `reset()`
- `clock()`

### Gerätekonfiguration

- `availableDeviceTypes()`
- `supportedDeviceNumbers()`
- `configurations()`
- `configuration()`
- `addableDeviceTypes()`
- `canAddDevice()`
- `addDevice()`
- `removeDevice()`
- `setConfiguration()`
- `restoreConfigurations()`

### Gerätefunktionen

- `isDiskImageLoaded()`
- `tearOffPrinter()`

### Ereignisse

- `takePrinterOutputEvents()`
- `takeDriveAccessEvents()`

Die Ereignisse werden als neutrale DTOs aus `IECDeviceEvent.h` bereitgestellt. Der Worker muss dafür weder `IECAsciiPrinter` noch `IECFoundryDrive` kennen.

Die endgültigen Rückgabetypen und Signaturen werden beim Entwurf der Schnittstellen festgelegt.

## 7. Konstruktion und Zerstörung

Die Komponenten werden im Worker in einer definierten Reihenfolge erzeugt:

1. `C64Machine`
2. `IECDeviceFactory`
3. `IECSystem`
4. `EmulationScheduler`

Die Konstruktorabhängigkeiten werden so gewählt, dass `IECSystem` den Bus an die Maschine anbinden und der Scheduler anschließend das System takten kann.

Die Zerstörung erfolgt in umgekehrter Reihenfolge:

1. `EmulationScheduler`
2. `IECSystem`
3. `IECDeviceFactory`
4. `C64Machine`

Dabei muss `IECSystem` vor der Factory zerstört werden, damit es keine Registrierungen oder Referenzen auf Geräte zurücklässt, die anschließend von der Factory freigegeben werden.

## 8. Betroffene Dateien

### Neu

- `C64/IEC/IECSystem.h`
- `C64/IEC/IECSystem.cpp`

### Anpassungen

- `Emulator/EmulatorWorker.h`
- `Emulator/EmulatorWorker.cpp`
- `Emulator/EmulationScheduler.h`
- `Emulator/EmulationScheduler.cpp`
- `C64/C64Machine.h/.cpp` nur falls die Bus-Anbindung angepasst werden muss
- `CMakeLists.txt`

Die Änderungen an `IECDeviceFactory`, `IECDeviceManager`, `IECProtocol` und den konkreten Geräten werden nur insoweit berücksichtigt, wie sie für die Schnittstelle von `IECSystem` erforderlich sind. Ihre eigentliche Umsetzung bleibt separaten Arbeitsschritten vorbehalten.

## 9. Nicht Bestandteil dieses Arbeitspakets

- Vollständiger Umbau von `IECProtocol`.
- Vollständiger Umbau von `IECDeviceManager`.
- Implementierung der `IECDeviceFactory`.
- Umbau der konkreten Drucker- und Disk-Geräte.
- Änderung des IEC-Handshakes oder der elektrischen Signalzeiten.
- Änderung des Disk- oder Druckerverhaltens.
- Neue IEC-Kommandos oder zusätzliche Gerätefunktionen.

## 10. Abnahmekriterien

- `EmulatorWorker` verwendet `IECSystem` als einzige IEC-Laufzeitschnittstelle.
- `IECSystem` besitzt Bus, Protokoll und Manager.
- `IECSystem` besitzt kein konkretes Drive und keine konkreten Geräte.
- Die Factory ist für Geräte und deren Backend-Ressourcen verantwortlich.
- Der Manager besitzt keine Geräte.
- Der Scheduler taktet das IEC-System genau einmal pro C64-Zyklus.
- Es gibt keine doppelten Protokoll- oder Geräte-Taktpfade.
- Drucker- und Drive-Events erreichen den Worker weiterhin über neutrale Event-Typen.
- Geräte werden vor ihrer Zerstörung aus dem Manager ausgetragen.
- Der Worker enthält keine Abhängigkeiten zu konkreten IEC-Geräteklassen.

## 11. Nächster Schritt

Vor der Implementierung werden die genauen Schnittstellen zwischen `IECSystem`, `IECDeviceFactory`, `IECDeviceManager` und `IECProtocol` abgestimmt.

Anschließend wird `IECSystem` mit seinen zentralen Infrastrukturkomponenten aufgebaut und der Scheduler auf den zentralen IEC-Taktpfad umgestellt.

