# CLAUDE.md - AI Assistant Guide for OKTET Labs Test Environment

## Project Overview

OKTET Labs Test Environment (TE) is a C-based automated testing framework for
creating and running test suites. It provides infrastructure for remote test
agent management, network traffic analysis, configuration management, and
comprehensive test result reporting. Licensed under Apache 2.0.

## Repository Structure

```
/
├── agents/           # Test agent implementations (unix, win32, proxy, power-ctl, switch-ctl)
├── apps/             # Engine applications (dpdkrpc, ta_core_watcher, ta_rpcprovider)
├── bpf/              # Berkeley Packet Filter support
├── conf/             # Configuration files (builder.conf, cs.conf, logger.conf, rcf.conf)
├── doc/              # Documentation sources (Sphinx RST, Doxygen resources, XSD schemas)
├── engine/           # Core engine components
│   ├── builder/      # Build system (te_meson_build, te_rpcgen, te_functions)
│   ├── configurator/ # Configuration server (OID-based, YAML/XML config)
│   ├── logger/       # Logging subsystem
│   ├── rcf/          # Remote Control Facility
│   └── tester/       # Test execution engine
├── include/          # Common header files (te_defs.h, logger_api.h, rpcgen.m4)
├── jenkins/          # CI/CD pipeline configuration (Groovy pipelines, helper scripts)
├── lib/              # Core libraries (67 subdirectories)
├── scripts/          # Build and utility scripts
├── suites/           # Test suites (selftest, selftad, clientserver, example, ipv6-demo)
├── tools/            # Utility tools (rgt, trc, tce, log_server, log_streaming)
├── dispatcher.sh     # Main build and run entry point
├── bootstrap.sh      # Cleanup script for fresh builds
├── meson.build       # Root Meson build configuration
├── meson_options.txt # Meson build options
├── Doxyfile          # Doxygen configuration
└── gen_doxygen       # Doxygen generation script
```

## Build System

### Meson (primary build system)

- **Version requirement**: >= 0.49.0
- **Default library type**: shared
- **Toolchain**: Flex (lexer), Bison (parser), M4 (macro processor for RPC definitions)
- **Root config**: `meson.build` + `meson_options.txt`

### Building

```sh
export TE_BASE=<path-to-te-sources>

# Build standalone TE
./dispatcher.sh

# Build with a test suite (from suite directory)
./run.sh
```

### Key Build Scripts

| Script | Purpose |
|--------|---------|
| `dispatcher.sh` | Main orchestrator - builds and starts TE |
| `bootstrap.sh` | Removes build artifacts for clean rebuilds |
| `engine/builder/te_meson_build` | Meson build wrapper (~1000 lines) |
| `engine/builder/te_cross_build_meson` | Cross-compilation support |
| `engine/builder/te_rpcgen` | RPC code generator |
| `engine/builder/te_functions` | Builder utility functions |
| `engine/builder/te_rcf_consistency_checks` | Consistency validation |

### Key Environment Variables

| Variable | Purpose |
|----------|---------|
| `TE_BASE` | TE source directory (auto-detected by dispatcher.sh) |
| `TE_BUILD` | Build workspace directory |
| `TE_INSTALL` | Installation directory |
| `TE_RUN_DIR` | Current run directory |
| `TE_EXT_LIBS` | External libraries location |

### System Dependencies

Install on Debian/Ubuntu:
```
libglib2.0-dev libxml2-dev libpopt-dev libssl-dev libcurl4-openssl-dev
libpcap-dev libnl-3-dev libtirpc-dev libjansson-dev libyaml-dev
libpcre2-dev expect-dev flex bison m4
```

## Key Libraries (`lib/`)

### Core
- `tools/` - Common utilities (strings, buffers, file ops, exec)
- `ipc/` - Inter-process communication
- `asn/` - ASN.1 support
- `ndn/` - Nested Data Notation
- `logic_expr/` - Logic expression evaluation

### Test API (TAPI)
- `tapi/` - Base Test API (~49K lines)
- `tapi_rpc/` - RPC wrapper functions
- `tapi_env/` - Test environment configuration
- `tapi_job/` - Job management API
- `tapi_tad/` - Traffic Application Domain support
- `tapi_cfg_*/` - Configuration wrappers (interfaces, aggregation, bridging, DHCP, etc.)
- `tapi_bpf/`, `tapi_dpdk/`, `tapi_nvme/` - Hardware-specific APIs
- `tapi_dns/`, `tapi_wifi/`, `tapi_upnp/` - Protocol-specific APIs

### RPC & Communication
- `rcfapi/`, `rcfpch/`, `rcfrpc/` - Remote Control Facility
- `rpctransport/`, `rpcclient/`, `rpcserver/` - RPC infrastructure
- `rpc_types/`, `rpcxdr/` - RPC type definitions and serialization

### Configuration
- `confapi/` - Configuration API
- `conf_oid/` - Configuration Object ID handling

### Agent Libraries
- `agentlib/` - Base agent library
- `ta_job/`, `ta_bpf/`, `ta_restconf/` - Agent-side implementations

### TAD (Test Argument Descriptor)
- `tad/` - 23 protocol subdirectories (arp, dhcp, eth, ipstack, iscsi, pcap, snmp, vxlan, etc.)

## Documentation Sources

TE has three documentation systems:

### 1. Doxygen API Documentation

Auto-generated from C header files (`.h`). This is the primary API reference.

- **Config**: `Doxyfile` (root)
- **Generate**: `./gen_doxygen`
- **Output**: `doc/generated/html/`
- **Entry point**: `lib/mainpage.dox` - defines main page linking to all API groups
- **Scope**: Parses headers in `lib/`, `engine/`, `include/`, `agents/`, `tools/rgt`, `tools/trc`
- **Module groups**: 253+ `@defgroup` definitions across the library headers
- **Additional Doxyfiles**: `lib/Doxyfile`, `lib/rcfpch/Doxyfile`, suite-specific Doxyfiles
- **Resources**: `doc/dox_resources/` contains:
  - `DoxygenLayout.xml` - Custom HTML layout
  - `doxygen.css` - Custom styling
  - `doxygenfilter` - Preprocessor for `@USER_EXPAND{}` directives
  - `tableofcontents` - Post-processor adding TOC to group pages
  - 52 image files (`.svg`, `.png`, `.ditaa`, `.graphml`)

### 2. Sphinx RST Documentation (User & Architecture Guides)

51 RST files providing user guides, architecture descriptions, and component documentation.

- **Location**: `doc/sphinx/pages/` (47 files), `doc/sphinx/index.rst`
- **Topics covered**:
  - TE user guide and build instructions (`group_te_user.rst`)
  - Engine components: Builder, Configurator, Dispatcher, RCF, Logger, Tester
  - Test Agent documentation (Unix, Windows)
  - Test API guides (TAPI configuration, network traffic, TCP states, RPC)
  - Traffic Application Domain (TAD)
  - Tools: RGT (Report Generator), TRC (Test Results Comparator)
  - Network namespaces, serial consoles, kernel logging, sniffer
  - ASN notation, scenario format, configuration model
- **Selftest docs**: `suites/selftest/ts/doc/` (3 RST files)
- **Static resources**: `doc/sphinx/static/` (CSS and images)
- **Pipeline**: Doxygen XML -> Doxyrest (XML-to-RST) -> Sphinx (RST-to-HTML)

### 3. Dox Files

- `lib/mainpage.dox` - Main Doxygen entry point with links to all library API groups
- `suites/ipv6-demo/tests/mainpage.dox` - IPv6 demo test documentation
- `suites/ipv6-demo/tests/sockets/package.dox` - IPv6 socket tests

### Other Documentation

- `README.md` - Project overview and build basics
- `README-Devel.md` - Developer guidelines (naming, error handling, memory management)
- `CONTRIBUTING.md` - Contribution guidelines (coding style, licensing, commit messages)
- `doc/cm/` - Configuration model documentation
- `doc/xsd/` - XML Schema definitions
- `doc/ndn/` - NDN format documentation
- `te_docgen.sh` - LaTeX/PDF documentation generator
- `scripts/doxyrest_deploy.sh` - Doxyrest deployment for Doxygen-to-Sphinx pipeline

## Coding Conventions

### Language: C (POSIX-focused)

Follow the [OKTET Labs C Style Guide](https://github.com/oktetlabs/styleguides/blob/master/cguide.md).

### C Formatting Quick Reference

**Indentation and line length:**
- 4 spaces per level (no tabs)
- Max 80 characters per line; aim for 60-70 in new code

**Braces** get their own line everywhere except `do...while`:

```c
/* Functions: return type on its own line, brace on its own line */
te_errno
te_example_func(int arg)
{
    ...
}

/* Control structures: brace on its own line */
if (rc != 0)
{
    ERROR("failed");
    return rc;
}
else
{
    INFO("ok");
}

/* Exception: do...while closing brace shares line with while */
do {
    rc = try_again();
} while (rc == TE_EAGAIN);

/* switch: case labels indented once from switch */
switch (op)
{
    case OP_ADD:
        result = a + b;
        break;

    default:
        result = 0;
        break;
}
```

**Function signatures** - return type on a separate line, parameters
aligned to the opening parenthesis when wrapping:

```c
static te_errno
long_function_name(const char *name, size_t len,
                   te_string *result)
{
    ...
}
```

**Pointers** - asterisk with the variable name, not the type:

```c
char   *ptr;
const char *fmt;
```

**Spacing:**
- Space after keywords (`if`, `for`, `while`, `switch`, `return`)
- Space around binary operators (`a + b`, `x == 0`, `i < n`)
- No space after unary operators (`!flag`, `*ptr`, `&var`, `-1`)
- No space before `(` in function calls: `func(arg)`
- Space after `,` in argument lists: `func(a, b, c)`

**Comments** - always `/* */`, never `//`:

```c
/* Single-line comment */

/*
 * Multi-line comment explaining
 * non-obvious logic.
 */
```

**Doxygen** - use `/** */` with `@` tags:

```c
/** @file
 * @brief Brief module description
 *
 * @defgroup group_name Human-Readable Name
 * @{
 */

/**
 * Brief function description.
 *
 * @param name    Parameter description.
 * @param[out] result  Output parameter.
 *
 * @return Status code.
 * @retval 0            Success.
 * @retval TE_ENOENT    Not found.
 */
te_errno
te_example_lookup(const char *name, int *result);

/** @} */
```

**Struct/enum formatting** - fields aligned, inline docs with `/**< */`:

```c
typedef struct te_example {
    char   *name;       /**< Human-readable name */
    size_t  size;       /**< Buffer size */
    bool    active;     /**< Whether currently in use */
} te_example;

typedef enum te_example_mode {
    TE_EXAMPLE_MODE_AUTO,       /**< Automatic selection */
    TE_EXAMPLE_MODE_MANUAL,     /**< Manual override */
} te_example_mode;
```

**Header guards** - double-underscore style:

```c
#ifndef __TE_EXAMPLE_H__
#define __TE_EXAMPLE_H__
...
#endif /* !__TE_EXAMPLE_H__ */
```

**Include ordering:**
1. `"te_config.h"` (always first in `.c` files)
2. System headers (`<stdio.h>`, `<sys/types.h>`)
3. Project headers (`"te_defs.h"`, `"logger_api.h"`)

**Macros** - uppercase, parameters in parentheses, trailing underscore
on parameter names to avoid shadowing:

```c
#define TE_EXAMPLE_INIT(size_) \
    { .name = NULL, .size = (size_), .active = false }
```

**One declaration per line, one statement per line.** Declare variables
where first used (C99 style is acceptable):

```c
size_t  len = strlen(name);
char   *copy = TE_STRDUP(name);
```

**File headers** - SPDX on line 1, then Doxygen file block with copyright:

```c
/* SPDX-License-Identifier: Apache-2.0 */
/** @file
 * @brief Brief description of the module
 *
 * Detailed description if needed.
 *
 * Copyright (C) 2024 OKTET Labs Ltd. All rights reserved.
 */
```

### Naming Prefixes (mandatory for externally visible entities)

| Prefix | Scope | Example Modules |
|--------|-------|-----------------|
| `te_` | TE-wide code | `lib/tools`, `include/te_defs.h` |
| `tapi_` | General TAPI functions | `lib/tapi*` |
| `tapi_cfg_` | TAPI wrappers around Configurator nodes | `lib/tapi/tapi_cfg_*` |
| `rpc_` | Engine-side RPC wrappers | `lib/tapi_rpc` |
| `cfg_` | Engine side of the Configurator | `engine/configurator`, `lib/confapi` |
| `ta_` | General agent-side code | `agents/*`, `lib/agentlib`, `lib/ta_*` |
| `rcf_` | Engine and agent-side RCF code | `engine/rcf`, `lib/rcf*` |
| `tad_` | Agent-side TAD code | `lib/tad` |
| `tester_` | Tester code | `engine/tester` |
| `trc_` | TRC-related code | `lib/trc`, `tools/trc` |

### Error Handling

- Return `te_errno` with a proper module code via `TE_RC(module, code)`, not negative `int`
- Compare errors with `TE_RC_GET_ERROR(rc) == TE_ENOENT`, never `rc == TE_ENOENT`
- Success is always `rc == 0` (no module code for zero)
- Propagate system errors with `TE_OS_RC()`, not ad hoc codes
- Use `assert()` for impossible conditions (logic errors)
- Use `TE_FATAL_ERROR()` for unrecoverable errors (e.g., OOM)
- Use `TEST_FAIL` / `CHECK_RC` in test code and high-level TAPI (never in agent-side code)
- Avoid excessive `NULL` pointer checks - prefer treating `NULL` as empty/black-hole

### Memory Management

Use TE allocation functions from `te_alloc.h` (never raw `malloc`/`calloc`/`strdup`):

| System Function | TE Equivalent |
|----------------|---------------|
| `malloc(x)` | `TE_ALLOC_UNINITIALIZED(x)` |
| `calloc(x, y)` | `TE_ALLOC(x * y)` |
| `strdup(s)` | `TE_STRDUP(s)` |
| `strndup(s, n)` | `TE_STRNDUP(s, n)` |
| `realloc(ptr, n)` | `TE_REALLOC(ptr, n)` |

TE allocators abort on OOM - no need to check for `NULL` return.

### Meson Build Files

- Use lowercase and underscore for word separation
- Use 4-space indentation
- No space after `(` / before `)`, spaces after `[` / before `]`

## Commit Message Format

### Summary Line

```
component: imperative description of change
```

- Max 60 characters, lowercase (except acronyms)
- Component is typically the second-level directory (e.g., `lib/asn`, `agent/unix`, `engine/tester`)
- Special components: `build`, `common`, `cs`, `doc`, `logger`, `rcf`, `revert`
- Start with imperative verb (fix, add, support, remove, update)
- No ticket references in summary

### Body

1. **Description** (optional): explain *why* the change is needed, wrap at 72 chars
2. **Helper trailers**: `Fixes:`, `OL-Redmine-Id:`, `Link:`, `Breaks:`, `Urgency:`
3. **Co-authorship**: `Reported-by:`, `Suggested-by:`, `Co-developed-by:`
4. **Signoff** (mandatory): `Signed-off-by:` (use `git commit -s`)
5. **Review** (mandatory): `Reviewed-by:`, `Acked-by:`, `Tested-by:`

### Example

```
lib/asn: fix build with GCC 12

The new compiler version produces warnings on implicit
function declarations which are now errors by default.

Fixes: afaada7087af ("Import the first public release")
Signed-off-by: John Doe <johnd@example.com>
Reviewed-by: Jane Doe <janed@example.com>
```

### Breaking Changes

Must include `Breaks:` trailer describing what broke. Breaking changes include:
new build dependencies, removal of modules/API elements, signature changes,
addition of function attributes.

## License

- All new code: **Apache 2.0**
- SPDX header required on first line (or second line for shebangs/XML/YAML)
- Copyright notice on line after SPDX tag

```c
/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2024 Your Organization. All rights reserved. */
```

## Architecture Quick Reference

### Engine Components

1. **Dispatcher** (`dispatcher.sh`): Orchestrates the build and starts all subsystems
2. **Builder** (`engine/builder/`): Compiles libraries, agents, and test suites via Meson
3. **Configurator** (`engine/configurator/`): Manages environment configuration via OID tree
4. **RCF** (`engine/rcf/`): Communicates with remote test agents
5. **Logger** (`engine/logger/`): Centralized logging from engine and agents
6. **Tester** (`engine/tester/`): Executes tests and manages results

### Test Agents

- **Unix** (`agents/unix/`): Primary agent for Linux/Unix systems
- **Win32** (`agents/win32/`): Windows agent
- **Proxy** (`agents/proxy/`): Proxy agent for test isolation
- **Power-ctl** (`agents/power-ctl/`): Device power management
- **Switch-ctl** (`agents/switch-ctl/`): Network switch control

### Tools

- **RGT** (`tools/rgt/`): Report Generator - log processing, indexing, formatting, bundling
- **TRC** (`tools/trc/`): Test Results Comparator - diff, filter, merge, report on test results
- **TCE** (`tools/tce/`): Test Configuration Engine

### Script Libraries

- `scripts/lib` - Core bash utilities (`ring()`, `warn()`, `fail()`, `try()`)
- `scripts/lib.grab_cfg` - Configuration reservation/release API
- `scripts/lib.site_opts` - Site-specific option handling
- `scripts/lib.meta` - Metadata generation

## Testing

### Self-Test Suites

- `suites/selftest/` - Core self-tests (11 test directories)
- `suites/selftad/` - TAD protocol testing
- `suites/clientserver/` - Client-server architecture tests
- `suites/example/` - Example test implementations

### Running Tests

```sh
# From a test suite directory
./run.sh

# Standalone build
./dispatcher.sh
```

## CI/CD

Jenkins pipelines in `jenkins/pipelines/`:
- `publish-logs` - Publishes test results (file storage or Artifactory)
- `te-doc` - Documentation generation
- `bublik-import` - Imports results to Bublik reporting

Helper scripts in `jenkins/scripts/`:
- `te_run_wrap` - Launches test processes preventing Jenkins kill
- `te_run_wait` - Monitors test process completion
- `te_run_wrap_read` - Reads process output

## Key Patterns for AI Assistants

1. **Always use TE naming prefixes** for new externally visible symbols
2. **Use `te_errno` + `TE_RC()`** for error returns, never bare negative integers
3. **Use `te_alloc.h`** functions, never raw `malloc`/`calloc`/`strdup`
4. **Check `README-Devel.md`** for detailed coding guidelines before making changes
5. **Follow `CONTRIBUTING.md`** for commit message format - signoff is mandatory
6. **Meson build files** use 4-space indent, lowercase with underscores
7. **SPDX + copyright header** required on all new files
8. **Legacy code exists** that doesn't follow current guidelines - prefer consistency with surrounding code over strict guideline adherence when modifying old code
9. **`TEST_FAIL` / `CHECK_RC`** must never be used in agent-side code
10. **Avoid excessive argument validation** - use proper types and `assert()` for logic errors instead of returning error codes for impossible conditions
