# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**SPTK (Simply Powerful Toolkit)** is a C++20 cross-platform library (Linux/Windows/BSD) providing utilities, networking, database abstraction, XML/JSON handling, threading primitives, web services, and an optional FLTK-based GUI layer. Version 5.6.6, licensed LGPL.

## Build System

CMake with Ninja. Pre-configured build directories already exist in the repo root:

| Directory | Type |
|---|---|
| `Debug/` | Debug build (default) |
| `Release/` | Release build |
| `DebugCoverage/` | Debug + gcov coverage |
| `cmake-build-debug-coverage/` | CLion coverage build |

### Build commands

```bash
# Build from an existing build directory
cd Debug && ninja

# Build a specific target
cd Debug && ninja sputil5

# Configure a fresh build directory (example: Debug)
cmake -B Debug -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Run all unit tests (from build dir)
cd Debug && ./test/sptk_unit_tests

# Run a specific test suite by name (GoogleTest filter)
cd Debug && ./test/sptk_unit_tests --gtest_filter="DateTime*"

# Install
cd Debug && ninja install
```

### Key CMake options

```
USE_FLTK       ON/OFF  GUI library (requires FLTK 1.3+, Cairo, libpng, libXext)
USE_MYSQL      ON/OFF  MySQL/MariaDB driver
USE_POSTGRESQL ON/OFF  PostgreSQL driver
USE_SQLITE3    ON/OFF  SQLite3 driver
USE_ODBC       ON/OFF  ODBC driver
USE_ORACLE     ON/OFF  Oracle OCI driver
USE_OPENSSL    ON/OFF  TLS/SSL support
USE_EPOLL      ON/OFF  Async socket events (epoll/kqueue/wepoll)
USE_GTEST      ON/OFF  Build unit tests
BUILD_EXAMPLES ON/OFF
BUILD_UTILS    ON/OFF
BUILD_WITH_COVERAGE ON/OFF  GCC --coverage
```

## Library Architecture

The build produces four shared libraries with a strict dependency order:

```
sputil5   — core utilities, threading, networking, xdoc (JSON/XML), JWT, tar, zlib/brotli
    └── spdb5   — database abstraction layer (Query, Transaction, ConnectionPool)
        └── sptk5   — FLTK GUI widgets (optional, only when USE_FLTK=ON)
        └── spwsdl5 — SOAP/REST web-service server (depends on sputil5 + spdb5)
```

### Source layout

```
src/sputil/   — implementation of sputil5
  core/       — Buffer, DateTime, String, Logger, RegularExpression, etc.
  net/        — TCP/UDP/SSL sockets, HTTP, SMTP, IMAP, Redis, SocketPool (epoll/kqueue)
  threads/    — Thread, ThreadPool, Timer, SmartLock, SynchronizedQueue, etc.
  xdoc/       — DOM-style JSON/XML Document/Node tree
  jwt/        — JWT encode/decode
  tar/        — Tar archive support
src/spdb/     — implementation of spdb5 (Query, Transaction, BulkQuery, ConnectionPool)
src/sptk/gui/ — implementation of sptk5 GUI widgets
src/drivers/  — pluggable DB drivers: MySQL, PostgreSQL, SQLite3, ODBC, Oracle, OracleOCI
src/wsdl/     — SOAP/REST web service server (WSServer, WSRequest, OpenApiGenerator)
```

### Public headers

All public headers live under `sptk5/` in the repo root (installed to `include/sptk5`).

Umbrella/convenience headers for common use:
- `<sptk5/cutils>` — core utilities
- `<sptk5/cnet>` — networking
- `<sptk5/cdatabase>` — DB pool + query
- `<sptk5/cthreads>` — threading primitives
- `<sptk5/cgui>` — GUI widgets

### Database layer

`DatabaseConnectionString` accepts URIs: `postgresql://user:pass@host:5432/db`, `mysql://...`, `sqlite3://localhost/path/to/file.sqlite3`, `oracle://...`, `mssql://dsn/db`.

`DatabaseConnectionPool` manages a pool of connections; `PoolDatabaseConnection` (RAII) borrows one. `Query` executes parameterised SQL statements. `Transaction` wraps RAII commit/rollback. DB drivers are loaded as shared libraries at runtime via `DriverLoaders`.

### Networking

`TCPServer` + `TCPServerListener` + `ThreadPool` form the async server backbone. `WSServer` extends `TCPServer` to handle SOAP/REST requests dispatched to `WSRequest`-derived handlers. `SocketPool` abstracts epoll (Linux), kqueue (BSD), and wepoll (Windows) for non-blocking I/O.

### GUI (FLTK)

All widgets inherit from `CControl` → `CLayoutClient` → `Fl_Group`. `CLayoutManager` handles declarative layout. Widget data binding uses `CDataControl` and can be connected to a `Query` result set via `CDBListView`.

### xdoc (JSON/XML)

`xdoc::Document` is a DOM tree of `xdoc::Node` objects. `ExportXML`/`ExportJSON` serialize, `ImportXML` parses. `NodeName` uses a pooled string to reduce allocation overhead.

## Testing

Tests use GoogleTest. The test binary is `test/sptk_unit_tests` (built into the build directory). Test source files are in `test/` and `test/tests/{core,net,spdb,threads,xdoc,jwt,tar,wsdl}/`.

Database tests require live DB servers. Connection strings are hard-coded in `test/sptk_unit_tests.cpp`; set up matching hosts (`dbhost_pg`, `dbhost_mysql`, etc.) or skip DB test suites with `--gtest_filter`.

## Code Conventions

- All public symbols are in `namespace sptk`. `xdoc` types are in `namespace sptk::xdoc`.
- `SP_EXPORT` macro marks symbols for shared-library export.
- Each file starts with the standard SPTK copyright/license block.
- C++20 throughout (`std::span`, structured bindings, concepts where appropriate).
- `#pragma once` used in all headers.
- RAII everywhere: locks via `std::scoped_lock`/`SmartLock`, DB connections via `PoolDatabaseConnection`, threads managed through `ThreadPool`/`WorkerThread`.
