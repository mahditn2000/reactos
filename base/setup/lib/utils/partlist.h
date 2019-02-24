/*
 * PROJECT:     ReactOS Setup Library
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Partition list functions
 * COPYRIGHT:   Copyright 2003-2019 Casper S. Hornstrup (chorns@users.sourceforge.net)
 *              Copyright 2018-2019 Hermes Belusca-Maito
 */

#pragma once

/* HELPERS FOR PARTITION TYPES **********************************************/

typedef struct _PARTITION_TYPE
{
    UCHAR Type;
    PCHAR Description;
} PARTITION_TYPE, *PPARTITION_TYPE;

#define NUM_PARTITION_TYPE_ENTRIES  143
extern PARTITION_TYPE PartitionTypes[NUM_PARTITION_TYPE_ENTRIES];


/* PARTITION UTILITY FUNCTIONS **********************************************/

typedef enum _FORMATSTATE
{
    Unformatted,
    UnformattedOrDamaged,
    UnknownFormat,
    Preformatted,
    Formatted
} FORMATSTATE, *PFORMATSTATE;

typedef struct _PARTENTRY
{
    LIST_ENTRY ListEntry;

    /* The disk this partition belongs to */
    struct _DISKENTRY *DiskEntry;

    /* Partition geometry */
    ULARGE_INTEGER StartSector;
    ULARGE_INTEGER SectorCount;

    BOOLEAN BootIndicator;
    UCHAR PartitionType;
    ULONG HiddenSectors;
    ULONG OnDiskPartitionNumber; /* Enumerated partition number (primary partitions first, excluding the extended partition container, then the logical partitions) */
    ULONG PartitionNumber;       /* Current partition number, only valid for the currently running NTOS instance */
    ULONG PartitionIndex;        /* Index in the LayoutBuffer->PartitionEntry[] cached array of the corresponding DiskEntry */

    WCHAR DriveLetter;
    WCHAR VolumeLabel[20];
    WCHAR FileSystem[MAX_PATH+1];
    FORMATSTATE FormatState;

    BOOLEAN LogicalPartition;

    /* Partition is partitioned disk space */
    BOOLEAN IsPartitioned;

/** The following three properties may be replaced by flags **/

    /* Partition is new, table does not exist on disk yet */
    BOOLEAN New;

    /* Partition was created automatically */
    BOOLEAN AutoCreate;

    /* Partition must be checked */
    BOOLEAN NeedsCheck;

} PARTENTRY, *PPARTENTRY;


typedef struct _BIOSDISKENTRY
{
    LIST_ENTRY ListEntry;
    ULONG DiskNumber;
    ULONG Signature;
    ULONG Checksum;
    BOOLEAN Recognized;
    CM_DISK_GEOMETRY_DEVICE_DATA DiskGeometry;
    CM_INT13_DRIVE_PARAMETER Int13DiskData;
} BIOSDISKENTRY, *PBIOSDISKENTRY;


typedef struct _DISKENTRY
{
    LIST_ENTRY ListEntry;

    /* Disk geometry */

    ULONGLONG Cylinders;
    ULONG TracksPerCylinder;
    ULONG SectorsPerTrack;
    ULONG BytesPerSector;

    ULARGE_INTEGER SectorCount;
    ULONG SectorAlignment;
    ULONG CylinderAlignment;

    /* BIOS parameters */
    BOOLEAN BiosFound;
    ULONG BiosDiskNumber;
//    ULONG Signature;  // Obtained from LayoutBuffer->Signature
//    ULONG Checksum;

    /* SCSI parameters */
    ULONG DiskNumber;
//  SCSI_ADDRESS;
    USHORT Port;
    USHORT Bus;
    USHORT Id;

    /* Has the partition list been modified? */
    BOOLEAN Dirty;

    BOOLEAN NewDisk; /* If TRUE, the disk is uninitialized */
    PARTITION_STYLE DiskStyle;  /* MBR/GPT-partitioned disk, or uninitialized disk (RAW) */

    UNICODE_STRING DriverName;

    PDRIVE_LAYOUT_INFORMATION LayoutBuffer;
    // TODO: When adding support for GPT disks:
    // Use PDRIVE_LAYOUT_INFORMATION_EX which indicates whether
    // the disk is MBR, GPT, or unknown (uninitialized).
    // Depending on the style, either use the MBR or GPT partition info.

    LIST_ENTRY PrimaryPartListHead; /* List of primary partitions */
    LIST_ENTRY LogicalPartListHead; /* List of logical partitions (Valid only for MBR-partitioned disks) */

    /* Pointer to the unique extended partition on this disk (Valid only for MBR-partitioned disks) */
    PPARTENTRY ExtendedPartition;

} DISKENTRY, *PDISKENTRY;


typedef struct _PARTLIST
{
    /*
     * Disk & Partition iterators.
     *
     * NOTE that when CurrentPartition != NULL, then CurrentPartition->DiskEntry
     * must be the same as CurrentDisk. We should however keep the two members
     * separated as we can have a current (selected) disk without any current
     * partition, if the former does not contain any.
     */
    PDISKENTRY CurrentDisk;
    PPARTENTRY CurrentPartition;

    /*
     * The system partition where the boot manager resides.
     * The corresponding system disk is obtained via:
     *    SystemPartition->DiskEntry.
     */
    PPARTENTRY SystemPartition;
    /*
     * The original system partition in case we are redefining it because
     * we do not have write support on it.
     * Please note that this is partly a HACK and MUST NEVER happen on
     * architectures where real system partitions are mandatory (because then
     * they are formatted in FAT FS and we support write operation on them).
     * The corresponding original system disk is obtained via:
     *    OriginalSystemPartition->DiskEntry.
     */
    PPARTENTRY OriginalSystemPartition;

    LIST_ENTRY DiskListHead;
    LIST_ENTRY BiosDiskListHead;

} PARTLIST, *PPARTLIST;

#define  PARTITION_TBL_SIZE 4

#define PARTITION_MAGIC     0xAA55

/* Defines system type for MBR showing that a GPT is following */
#define EFI_PMBR_OSTYPE_EFI 0xEE

#include <pshpack1.h>

typedef struct _PARTITION
{
    unsigned char   BootFlags;        /* bootable?  0=no, 128=yes  */
    unsigned char   StartingHead;     /* beginning head number */
    unsigned char   StartingSector;   /* beginning sector number */
    unsigned char   StartingCylinder; /* 10 bit nmbr, with high 2 bits put in begsect */
    unsigned char   PartitionType;    /* Operating System type indicator code */
    unsigned char   EndingHead;       /* ending head number */
    unsigned char   EndingSector;     /* ending sector number */
    unsigned char   EndingCylinder;   /* also a 10 bit nmbr, with same high 2 bit trick */
    unsigned int  StartingBlock;      /* first sector relative to start of disk */
    unsigned int  SectorCount;        /* number of sectors in partition */
} PARTITION, *PPARTITION;

typedef struct _PARTITION_SECTOR
{
    UCHAR BootCode[440];                     /* 0x000 */
    ULONG Signature;                         /* 0x1B8 */
    UCHAR Reserved[2];                       /* 0x1BC */
    PARTITION Partition[PARTITION_TBL_SIZE]; /* 0x1BE */
    USHORT Magic;                            /* 0x1FE */
} PARTITION_SECTOR, *PPARTITION_SECTOR;

#include <poppack.h>

typedef struct
{
    LIST_ENTRY ListEntry;
    ULONG DiskNumber;
    ULONG Identifier;
    ULONG Signature;
} BIOS_DISK, *PBIOS_DISK;



ULONGLONG
AlignDown(
    IN ULONGLONG Value,
    IN ULONG Alignment);

ULONGLONG
AlignUp(
    IN ULONGLONG Value,
    IN ULONG Alignment);

ULONGLONG
RoundingDivide(
   IN ULONGLONG Dividend,
   IN ULONGLONG Divisor);



PPARTLIST
CreatePartitionList(VOID);

VOID
DestroyPartitionList(
    IN PPARTLIST List);

PDISKENTRY
GetDiskByBiosNumber(
    IN PPARTLIST List,
    IN ULONG BiosDiskNumber);

PDISKENTRY
GetDiskByNumber(
    IN PPARTLIST List,
    IN ULONG DiskNumber);

PDISKENTRY
GetDiskBySCSI(
    IN PPARTLIST List,
    IN USHORT Port,
    IN USHORT Bus,
    IN USHORT Id);

PDISKENTRY
GetDiskBySignature(
    IN PPARTLIST List,
    IN ULONG Signature);

PPARTENTRY
GetPartition(
    // IN PPARTLIST List,
    IN PDISKENTRY DiskEntry,
    IN ULONG PartitionNumber);

BOOLEAN
GetDiskOrPartition(
    IN PPARTLIST List,
    IN ULONG DiskNumber,
    IN ULONG PartitionNumber OPTIONAL,
    OUT PDISKENTRY* pDiskEntry,
    OUT PPARTENTRY* pPartEntry OPTIONAL);

BOOLEAN
SelectPartition(
    IN PPARTLIST List,
    IN ULONG DiskNumber,
    IN ULONG PartitionNumber);

PPARTENTRY
GetNextPartition(
    IN PPARTLIST List);

PPARTENTRY
GetPrevPartition(
    IN PPARTLIST List);

BOOLEAN
CreatePrimaryPartition(
    IN PPARTLIST List,
    IN PPARTENTRY SelectedEntry,
    IN ULONGLONG SectorCount,
    IN BOOLEAN AutoCreate);

BOOLEAN
CreateExtendedPartition(
    IN PPARTLIST List,
    IN PPARTENTRY SelectedEntry,
    IN ULONGLONG SectorCount);

BOOLEAN
CreateLogicalPartition(
    IN PPARTLIST List,
    IN PPARTENTRY SelectedEntry,
    IN ULONGLONG SectorCount,
    IN BOOLEAN AutoCreate);

VOID
DeletePartition(
    IN PPARTLIST List,
    IN PPARTENTRY PartEntry);

VOID
DeleteCurrentPartition(
    IN PPARTLIST List);

VOID
CheckActiveSystemPartition(
    IN PPARTLIST List);

NTSTATUS
WritePartitions(
    IN PDISKENTRY DiskEntry);

BOOLEAN
WritePartitionsToDisk(
    IN PPARTLIST List);

BOOLEAN
SetMountedDeviceValue(
    IN WCHAR Letter,
    IN ULONG Signature,
    IN LARGE_INTEGER StartingOffset);

BOOLEAN
SetMountedDeviceValues(
    IN PPARTLIST List);

VOID
SetPartitionType(
    IN PPARTENTRY PartEntry,
    IN UCHAR PartitionType);

ERROR_NUMBER
PrimaryPartitionCreationChecks(
    IN PPARTENTRY PartEntry);

ERROR_NUMBER
ExtendedPartitionCreationChecks(
    IN PPARTENTRY PartEntry);

ERROR_NUMBER
LogicalPartitionCreationChecks(
    IN PPARTENTRY PartEntry);

BOOLEAN
GetNextUnformattedPartition(
    IN PPARTLIST List,
    OUT PDISKENTRY *pDiskEntry OPTIONAL,
    OUT PPARTENTRY *pPartEntry);

BOOLEAN
GetNextUncheckedPartition(
    IN PPARTLIST List,
    OUT PDISKENTRY *pDiskEntry OPTIONAL,
    OUT PPARTENTRY *pPartEntry);

/* EOF */
