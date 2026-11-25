
# Work Package: Foundry Drive – IEC Drive and DOS Implementation

**Status:** Planned  
**Last updated:** 2026-11-25

## 1. Goal

Implement the IEC-facing behavior of `IECFoundryDrive` so that an emulated C64 can use the Foundry Drive as a practical D64-based disk drive.

The implementation should support standard C64 disk operations, including:

- Opening and closing IEC channels.
- Loading the disk directory.
- Loading PRG files from a disk image.
- Reading drive status and DOS error messages.
- Writing PRG files to a disk image using the C64 `SAVE` command.
- Reporting disk and filesystem errors correctly.

The target is practical compatibility with standard C64 software using normal IEC drive operations.

The internal CPU, VIA chips and physical electronics of a 1541 drive are not part of this work package.

The completed IEC architecture is the foundation. This work package implements the actual drive behavior on top of that architecture.

## 2. Current implementation findings

The current repository implementation has been inspected.

### 2.1 IECFoundryDrive

`IECFoundryDrive` derives from `IECDevice`.

Implemented:

- Owns and manages its `IECFoundryDisk` objects.
- Maps IEC device numbers to disk slots.
- Supports disk enable/disable.
- Supports image insertion, ejection and saving.
- Collects disk image events.
- Collects sector-access events.
- Reports device events through `takeEvents()`.

Not implemented:

- `processEvent()` currently does not process IEC events.
- No OPEN/CLOSE handling.
- No secondary-address or channel management.
- No IEC data transmission for directory or file contents.
- No IEC data reception for file writes.
- No DOS command processing.
- No drive status channel.

### 2.2 IECFoundryDisk

`IECFoundryDisk` is an internal component owned by `IECFoundryDrive`.

Implemented:

- Image insertion and ejection.
- Image persistence.
- Image validity and modification state.
- Sector reading.
- Directory-program generation.
- PRG file lookup and reading.
- Image and sector-access events.

Not implemented:

- Sector writing.
- Creating PRG files.
- Updating directory entries.
- Updating the BAM.
- Deleting or replacing existing files.

The disk object has no IEC device number and does not process IEC protocol events.

### 2.3 Existing protocol interface

The central IEC protocol and generic device architecture are implemented.

`IECProtocolEvent` currently distinguishes:

- `DeviceSelected`
- `SecondaryAddressReceived`
- `Open`
- `Close`
- `ByteReceived`
- `EndOfTransmission`
- `DeviceDeselected`

The secondary address is preserved in the event.

`IECProtocolResponse` currently supports:

- `None`
- `SendData`

The protocol owns electrical bus handling, timing and data transmission. The drive must not implement bus-level signaling itself.

### 2.4 Existing event and request infrastructure

The generic request/response infrastructure is implemented.

Device-originated events are forwarded through `IECDeviceManager`, `IECSystem` and `EmulatorWorker`.

Existing drive events include:

- Image inserted/ejected/invalid.
- Image errors.
- Sector read/write activity and errors.

The existing GUI-facing image-management functionality must remain operational.

## 3. Architecture and responsibilities

### 3.1 IECFoundryDrive

`IECFoundryDrive` remains the registered IEC device.

It is responsible for:

- Processing IEC protocol events.
- Mapping the addressed IEC number to the corresponding disk slot.
- Managing IEC channels.
- Handling OPEN and CLOSE.
- Interpreting drive commands.
- Managing data transfers between the C64 and the disk.
- Handling the DOS command channel.
- Producing appropriate IEC protocol responses.
- Translating disk/storage errors into DOS status information.

The drive owns the IEC-facing state and command logic.

It must not access the IEC bus or central protocol directly.

### 3.2 IECFoundryDisk

`IECFoundryDisk` remains the storage component.

It is responsible for:

- Accessing image data.
- Reading and writing sectors.
- Reading directory information.
- Reading PRG files.
- Creating and updating directory entries.
- Updating the BAM.
- Managing image modification state.
- Reporting storage-related errors and sector activity.

It must not know about IEC channels, OPEN/CLOSE commands or protocol timing.

### 3.3 IECDeviceManager and IECSystem

No architectural redesign is intended.

The existing generic event and response path remains unchanged unless implementation reveals a concrete missing capability in the generic interface.

The manager must remain independent of drive-specific behavior.

### 3.4 GUI

The existing GUI remains responsible for:

- Drive configuration.
- Disk slot enable/disable.
- Image insertion and ejection.
- Displaying image state and errors.
- Displaying drive activity.

No direct access from the GUI to live drive or disk objects is introduced.

Any additional GUI-facing information must use the existing device-event or request/response infrastructure.

## 4. Functional scope

### 4.1 IEC channel handling

Implement drive-side handling for:

- OPEN.
- CLOSE.
- Secondary addresses.
- Channel allocation and release.
- Data transfer associated with an opened channel.
- Correct handling of invalid or unavailable channels.

The implementation must distinguish between command, data and status channels.

Channel state belongs to the addressed drive instance.

The behavior must support multiple independent Foundry Drive instances.

### 4.2 Directory loading

Support the standard directory load operation:

`LOAD"$",8`

The drive must:

- Generate the directory program from the addressed disk.
- Transfer the generated data through IEC.
- Preserve the expected C64 load-address and directory-entry format.
- Handle empty directories.
- Handle invalid or missing images.
- Report directory and sector errors correctly.

The existing `IECFoundryDisk::createDirectoryProgram()` implementation should be inspected and reused where appropriate.

### 4.3 PRG loading

Support loading PRG files through normal C64 IEC operations.

Examples include:

- `LOAD"PROGRAM",8`
- Loading with a specified secondary address where applicable.
- Loading files by name.
- Supporting the existing filename normalization and wildcard behavior where compatible.

The drive must:

- Resolve the requested file.
- Read its data from the disk.
- Transfer the load address and program data correctly.
- Handle EOI and transfer completion correctly.
- Report missing files and storage errors through DOS status.

The existing `IECFoundryDisk::readFile()` implementation should be inspected and reused where appropriate.

### 4.4 DOS status channel

Implement a DOS status channel, normally addressed through secondary address 15.

The status must provide the expected Commodore-style information, including:

- Error number.
- Error description.
- Track.
- Sector.

The drive must maintain and report status consistently.

Successful operations must update the status appropriately.

Failed operations must provide useful DOS error information.

The GUI-facing error events remain separate from the C64-visible DOS status.

### 4.5 DOS command handling

Implement the command-channel handling required by the supported operations.

The command parser must be separated from the low-level sector storage operations.

The initial command set should focus on commands required for the agreed LOAD/SAVE functionality.

Additional DOS commands may be added later without changing the IEC architecture.

### 4.6 Disk writing and SAVE

Implement the storage functionality required to write PRG files to a D64 image.

This includes:

- Writing sectors.
- Allocating free sectors.
- Updating the BAM.
- Creating directory entries.
- Writing file data and sector links.
- Updating the image modification state.
- Preserving image consistency when an operation fails.

The target operation is a normal C64 `SAVE` command to the drive.

The implementation must handle insufficient disk space and other write failures without leaving inconsistent directory or BAM information.

The existing image persistence behavior must remain intact.

### 4.7 Image validation and errors

Image handling must distinguish between:

- No image configured.
- Image configured but not readable.
- Image readable but invalid or unsupported.
- Valid image.

The existing `hasImage()` behavior remains based on whether an image is configured, not whether it could be loaded successfully.

Image-invalid and error notifications remain events.

Replacing an image must continue to replace the previous image even when the new image is invalid.

Drive operations must not silently treat an invalid image as a valid disk.

The exact supported D64 image sizes and validation rules must be established during implementation.

## 5. Configuration and lifecycle constraints

The existing drive and disk configuration model is retained.

Important existing behavior:

- The drive reserves its complete configured IEC address range.
- The drive is registered under every address in that range.
- Disk objects do not have IEC addresses of their own.
- The drive maps IEC addresses to disk slots.
- Disk enable/disable and image changes are applied to the existing disk configuration.
- Structural changes to start address or slot count rebuild disk objects.

Do not redesign the configuration model as part of this work package unless a concrete functional requirement makes it necessary.

The existing save-on-exit and image persistence semantics must be preserved.

Reset must clear transient IEC/channel state without unintentionally changing the configured disk or image assignments.

## 6. Implementation sequence

The implementation should proceed in coherent, buildable steps.

### Step 1: Define drive-side protocol handling

- Inspect the current event and response semantics.
- Define the drive's handling of selection, secondary address, OPEN, CLOSE and received bytes.
- Define how the drive provides data to the central protocol.
- Define how transfer completion and EOI are handled.
- Identify any actual generic protocol interface deficiencies before changing shared architecture.

### Step 2: Implement channel management

- Introduce the minimum required channel state.
- Handle OPEN and CLOSE.
- Distinguish command, data and status channels.
- Handle invalid channel operations.
- Ensure reset clears transient channel state.

### Step 3: Implement directory transfer

- Connect directory generation to the IEC transfer path.
- Support `LOAD"$",8`.
- Verify data format and transfer completion.
- Handle errors and empty directories.

### Step 4: Implement PRG loading

- Connect file lookup and reading to IEC channel handling.
- Transfer PRG data correctly.
- Verify load address, data length and EOI behavior.
- Report missing files and read errors through DOS status.

### Step 5: Implement DOS status

- Introduce drive status state.
- Implement status-channel reads.
- Translate storage and protocol-level operation failures into DOS status.
- Verify status after successful and failed operations.

### Step 6: Implement sector writing

- Implement `IECFoundryDisk::writeSector()`.
- Preserve sector access event behavior.
- Update image modification state.
- Handle invalid tracks, sectors and image boundaries.
- Verify that failed writes do not corrupt image data.

### Step 7: Implement PRG file creation and SAVE

- Implement free-sector allocation.
- Update BAM information.
- Create directory entries.
- Write file data and sector chains.
- Support the agreed C64 SAVE operation.
- Handle existing filenames and insufficient disk space.
- Ensure image persistence remains correct.

### Step 8: Integration and compatibility testing

- Test all supported operations through the actual IEC protocol path.
- Verify drive and disk events reach the GUI.
- Verify DOS status.
- Verify multiple drives and different IEC addresses.
- Test with real C64 programs where possible.

## 7. Acceptance criteria

The work package is complete when:

- The drive processes the existing generic IEC protocol events.
- OPEN and CLOSE are handled.
- Secondary addresses and channels are managed correctly.
- `LOAD"$",8` works through IEC.
- PRG files can be loaded through IEC.
- The DOS status channel works.
- Relevant DOS commands are parsed and handled.
- D64 sectors can be written.
- PRG files can be saved through IEC.
- BAM and directory information remain consistent after writes.
- Insufficient space and invalid images produce appropriate errors.
- Image and sector-access events continue to reach the GUI.
- Existing image insertion, ejection, enable/disable and persistence behavior is preserved.
- Multiple independent drives continue to work.
- The central IEC protocol remains the sole owner of bus signaling and timing.

## 8. Explicit non-goals

- Emulation of the internal 1541 CPU.
- Emulation of VIA chips.
- GCR encoding and decoding.
- Rotational timing and physical disk mechanics.
- Copy protection and non-standard fast loaders.
- Replacing the central IEC protocol.
- Introducing per-device protocol instances.
- Making `IECFoundryDisk` an `IECDevice`.
- Moving disk ownership into `IECSystem` or `IECDeviceManager`.
- Reworking the existing drive configuration or GUI without a concrete requirement.
- Implementing every command supported by a physical Commodore DOS at once.

## 9. Open questions

The following details should be decided before or during the corresponding implementation steps:

- Which D64 image sizes and variants are supported initially?
- Which DOS commands are required for the first usable LOAD/SAVE release?
- How should saving a file with an existing filename behave?
- Which file types beyond PRG should be supported?
- How should partial write failures be handled and reported?
- Which real C64 test programs should form the compatibility baseline?

These questions must not block the initial protocol and channel design, but must be resolved before implementing the affected functionality.

## 10. Working rules

- The current repository is the source of truth.
- Inspect the actual source before every implementation step.
- Preserve the completed IEC architecture.
- Do not introduce unrelated refactoring.
- Keep drive command logic in `IECFoundryDrive`.
- Keep image and sector operations in `IECFoundryDisk`.
- Avoid unnecessary abstractions.
- Keep the emulation-cycle path efficient.
- Build and test after each coherent step.
- The repository owner applies, builds, tests and commits all changes.

