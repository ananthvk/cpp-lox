# Bytecode file format specification

All values are stored in Little endian byte order

A compiled Lox program contains a file header, followed by a list of chunks, followed by the global symbol table, and the string pool

## High level design

```
┌──────────────┬─────────┬─────────┬───────┬──────────────┬─────────┐
│  File header │ Chunk 0 │ Chunk 1 │ ..... │ Symbol table │ Strings │
└──────────────┴─────────┴─────────┴───────┴──────────────┴─────────┘
```

Each chunk is composed of a chunk header, constant pool, the bytecode instructions, and optionally debug info (which contains bytecode offset to line mapping)

```
┌──────────────┬───────────────┬───────────────────────┬──────────────────────┐
│ Chunk header │ Constant pool │ Bytecode instructions │ Debug info (optional)│
└──────────────┴───────────────┴───────────────────────┴──────────────────────┘
```

The symbol table is serialized as symbol table header, followed by values. 

```
┌─────────────────────────┬─────────┬─────────┬───────┬─────────┐
│ Symbol table header     │ Value  0│ Value  1│ ..... │ Value  N│
└─────────────────────────┴─────────┴─────────┴───────┴─────────┘
```

Strings are stored in a compact representation, with the header first (which includes the number of strings stored), followed by the strings. Each string is encoded in a length-prefixed format. The size of the string is written first (`4 bytes` unsigned integer), then the characters of the string

```
┌─────────────────────────┬─────────────────────────┬───────┬─────────────────────────┐
│ Strings header          │ String 0                │ ..... │ String N                │
└─────────────────────────┴─────────────────────────┴───────┴─────────────────────────┘
                          │[size][characters]       │       │[size][characters]       │
```

## Detailed design

### File header (32 bytes)

| Name                | Offset | Size (bytes) | Type            | Comments                      |
|---------------------|--------|--------------|-----------------|-------------------------------|
| Magic number        | 0      | 4            |`0xC 0x0 0xD 0xE`| Identifies the file           |
| CRC                 | 4      | 4            | uint32_t        | CRC32 checksum of full file   |
| Version major       | 8      | 1            | uint8_t         | Major version number          |
| Version minor       | 9      | 1            | uint8_t         | Minor version number          |
| Version patch       | 10     | 1            | uint8_t         | Patch version number          |
| Number of chunks    | 11     | 2            | uint16_t        | Count of chunks               |
| Chunks start offset | 13     | 4            | uint32_t        | Offset to first chunk         |
| Global table offset | 17     | 4            | uint32_t        | Offset to global symbol table |
| Strings offset      | 21     | 4            | uint32_t        | Offset to string pool         |
| File size           | 25     | 4            | uint32_t        | Total size of file            |
| Reserved            | 29     | 3            | Reserved        | Reserved for future use       |

### Chunk

#### Chunk header (16 bytes)

| Name                    | Offset | Size (bytes) | Type     | Comments                           |
|-------------------------|--------|--------------|----------|------------------------------------|
| Type                    | 0      | 1            | char     | Chunk identifier, set to 'F'       |
| Function name index     | 1      | 4            | uint32_t | Index into constant strings        |
| Arity                   | 5      | 1            | uint8_t  | Number of function arguments       |
| Upvalue count           | 6      | 2            | uint16_t | Number of upvalues                 |
| Constant pool size      | 8      | 2            | uint16_t | Size of constant pool              |
| Code length             | 10     | 4            | uint32_t | Length of bytecode instructions    |
| Debug info present      | 14     | 1            | uint8_t  | 1 if debug info present, 0 if not  |
| Reserved                | 15     | 1            | Reserved | Reserved for future use            |

#### Constant pool (variable size)

The constant pool contains 0 or more constants used by the function, they are stored as 9 byte records

| Name  | Offset | Size (bytes) | Type    | Comments                                        |
|-------|--------|--------------|---------|-------------------------------------------------|
| Type  | 0      | 1            | uint8_t | Constant type identifier                        |
| Value | 1      | 8            | varies  | Constant value (depends upon the type)          |

Each constant used is stored as a fixed record, the total size of the constant pool is calculated by multiplying the constant pool size (from chunk header) with 9 bytes.

#### Bytecode instructions (variable size)

Contains the compiled bytecode of this chunk, that needs to be executed. Stored as a sequence of bytes, the number of instructions is stored in the chunk header.

#### Debug info (variable size, optional)

| Name                    | Offset | Size (bytes) | Type     | Comments                           |
|-------------------------|--------|--------------|----------|------------------------------------|
| Number of pairs         | 0      | 4            | uint32_t | Count of offset/line number pairs  |
| Offset/Line pairs       | 4      | variable     | varies   | Sequence of offset/line pairs      |

Each offset/line pair is 8 bytes:
- Bytecode offset (4 bytes, uint32_t)
- Line number (4 bytes, uint32_t)

The total size of debug info can be calculated by multiplying the number of pairs with 8 bytes

### Symbol table

#### Symbol table header (8 bytes)

| Name                    | Offset | Size (bytes) | Type     | Comments                           |
|-------------------------|--------|--------------|----------|------------------------------------|
| Size                    | 0      | 4            | uint32_t | Number of entries in global table  |
| Reserved                | 4      | 8            | Reserved | Reserved                           |

#### Values (variable size)

The global symbol table contains 0 or more values used by any function in the program, they are stored as 24 byte records

| Name        | Offset | Size (bytes) | Type     | Comments                                       |
|-------------|--------|--------------|----------|------------------------------------------------|
| Name        | 0      | 4            | uint32_t | String index for symbol name                   |
| Index       | 4      | 4            | uint32_t | Symbol index                                   |
| Type        | 8      | 1            | uint8_t  | Value type identifier                          |
| Value       | 9      | 8            | varies   | Value data (depends upon the type)             |
| Defined     | 17     | 1            | uint8_t  | 1 if defined, 0 if not                         |
| Initialized | 18     | 1            | uint8_t  | 1 if initialized, 0 if not                     |
| Is const    | 19     | 1            | uint8_t  | 1 if constant, 0 if not                        |
| Reserved    | 20     | 1            | uint8_t  | Reserved for future use                        |
| Reserved    | 21     | 3            | Reserved | Reserved for future use                        |

The total size of the symbol table can be found by multiplying size (from header) with 24 bytes

### Strings

#### String header (8 bytes)

| Name                    | Offset | Size (bytes) | Type     | Comments                           |
|-------------------------|--------|--------------|----------|------------------------------------|
| Size                    | 0      | 4            | uint32_t | Number of strings stored           |
| Reserved                | 4      | 8            | Reserved | Reserved                           |

#### String values (variable size)

Each string is stored as a length-prefixed value:

| Name       | Size (bytes) | Type     | Comments                    |
|------------|--------------|----------|-----------------------------|
| Length     | 4            | uint32_t | Size of string in bytes     |
| Characters | variable     | char[]   | String data                 |

Strings are stored sequentially from string 0 to string N, where N is determined by the size field in the string header.