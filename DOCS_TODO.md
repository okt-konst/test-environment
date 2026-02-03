# Documentation Update Plan

Audit of all 47 Sphinx RST files in `doc/sphinx/pages/` against the current
codebase. Organized by priority.

---

## 1. Systemic Issues (affect many files)

### 1.1 Wrong code-block language tags -- DONE

~~50+ code blocks use `.. ref-code-block:: cpp` for XML, YAML, or C content.~~

All fixed. Changed `cpp` to the correct language (`xml`, `yaml`, `c`, `shell`,
or `none`) across all 10 files plus 5 additional files found during the fix:

- `group_sniffer.rst` (15 blocks)
- `group_serial.rst` (18 blocks)
- `group_kernel_log.rst` (4 blocks)
- `group_console_ll.rst` (4 blocks)
- `group_te_cmd_monitor.rst` (7 blocks)
- `group_te_netns.rst` (4 blocks)
- `group_te_scenarios.rst` (2 blocks)
- `group_csap.rst` (1 block)
- `group_route_gw.rst` (3 blocks)
- `group_tcp_sock_emulation.rst` (1 block)
- `group_te_ts.rst` (18 blocks)
- `group_te_user.rst` (2 blocks)
- `group_te_user_run_details.rst` (3 blocks)

### 1.2 Outdated XML file references (should be YAML)

Configuration model files were renamed from `.xml` to `.yml`.

| File | Line | Current | Correct |
|------|------|---------|---------|
| `group_te_engine_conf.rst` | 79 | `cm_base.xml` | `cm_base.yml` |
| `group_te_agents_win.rst` | 57 | `cm_base.xml` | `cm_base.yml` |

---

## 2. Incorrect Content (HIGH priority)

### 2.1 group_te_agents.rst

- **Line 31**: Claims FreeBSD support, but `agents/unix/conf/daemons/conf_daemons_internal.h`
  has `#error FreeBSD is not supported yet`. Remove or qualify the FreeBSD claim.
- **Lines 135-139**: States "two main types of te_agents" (Unix, Windows), but there
  are five agent types: unix, win32, proxy, power-ctl, switch-ctl. Document or
  at least mention the other three.

### 2.2 group_te_engine_dispatcher.rst

- **Line 135**: Option `force` missing `--` prefix (should be `--force`).
- **Lines 313, 316**: Options `test-wof` and `test-woc` missing `--` prefix.
- **Line 388**: Incomplete default value: `sniff-snaplen ... By default: .`
  (truncated). Actual default is unlimited.
- **Line 403**: Hardcoded developer path `/home/aizrailev/te/caps`. Should be
  `${TE_RUN_DIR}/caps`.
- **Line 132**: Malformed usage line with spurious `=""` XML-like attributes.

### 2.3 group_te_engine_rcf.rst

- **Lines 428-448**: RCF configuration examples use `confstr="..."` XML attribute
  syntax, but the actual format uses `<conf name="...">value</conf>` elements.

### 2.4 group_trc.rst

- **Line 75**: "Failed, as expected" row description says "tests which were
  expected to pass and passed" - contradicts the label.

### 2.5 group_tapi_tad_main.rst

- **Line 35**: References non-existent document `doc/hld/OKTL-HLD-0000236-TE_TAD.sxw`.
  The `doc/hld/` directory does not exist.

### 2.6 group_tapi_conf.rst

- **Line 66**: References non-existent Doxygen group `tapi_conf_eth`. No
  `@defgroup tapi_conf_eth` exists. Should reference `tapi_conf_if` or
  `tapi_conf_iface`.

### 2.7 group_te_agents_conf.rst

- **Lines 239, 241**: Code example uses `strdup()` (should be `TE_STRDUP()`) and
  is missing `return` before `TE_RC(TE_TA_UNIX, TE_ENOMEM)`.

### 2.8 group_te_agents_unix.rst

- **Lines 64-80**: Claims `agents/unix/rpc` directory exists for RPC implementation,
  but no such directory exists. Missing documentation of subdirectories: ovs,
  process, rule, tc, vm.

---

## 3. Broken References and Links -- DONE

All 7 instances fixed:

- `group_rgt.rst`: filled empty cross-references with `:ref:` links to
  "Logs processing" and "Log bundle" sections
- `group_te_user.rst`: replaced `@te_user_run` with `:ref:` to TE Execution
- `group_console_ll.rst`: converted MediaWiki `[[...]]` and HTML `<note>` to
  RST `.. note::` directive with `:ref:` cross-reference
- `page_loggerten.rst`: replaced hardcoded HTML link with `:ref:` anchor
- `group_tapi_tad_main.rst`: commented out reference to non-existent HLD
  document; toctree path for `group_tapi_tad_ipstack.rst` is correct (file
  is in `pages/`, not `generated/`)
- `group_tapi_wifi.rst`: added missing `tapi_wifi_wpa_cli` module to toctree

---

## 4. Typos and Spelling Errors

| File | Line | Error | Correction |
|------|------|-------|------------|
| `group_te_engine_rcf.rst` | 261, 263 | "possile" | "possible" |
| `group_te_engine_builder.rst` | 197 | "reffered" | "referred" |
| `group_te_engine_tester.rst` | 180, 282 | "reffered" | "referred" |
| `group_te_agents.rst` | 31 | "FreBSD" | "FreeBSD" |
| `group_te_agents_unix.rst` | 25 | "Unix Test Egent" | "Unix Test Agent" |
| `group_te_agents_conf.rst` | 109, 252, 316 | "bother node" | "brother node" |
| `group_te_agents_conf.rst` | 236 | "natuarally" | "naturally" |
| `group_te_agents_conf.rst` | 246 | "synamic" | "dynamic" |
| `group_ta_core_watcher.rst` | 81 | "simultanously" | "simultaneously" |
| `group_te_lib_tapi_conf_net.rst` | 21 | "phisical" | "physical" |
| `group_te_tools.rst` | 51 | "Dymanic array" | "Dynamic array" |
| `page_asn.rst` | 19 | "nevetheless" | "nevertheless" |
| `add_doc.rst` | 26 | "doc/shpinx/pages" | "doc/sphinx/pages" |

---

## 5. Formatting Issues

| File | Line | Problem |
|------|------|---------|
| `group_te_engine_dispatcher.rst` | 39 | Missing separator between `TE_INSTALL_SUITE` and its description. |
| `group_te_netns.rst` | 238 | Malformed inline markup: `tapi_cfg_base_if\_\*` has incorrect escaping. |
| `group_terminology.rst` | 26 | "DUT" label missing from table row. |
| `group_confapi.rst` | 30, 54 | Informal text reference instead of `:ref:` cross-reference. |

---

## 6. Missing Documentation (NEW pages needed)

### 6.1 Missing TAPI module pages (HIGH priority)

These libraries have significant implementations (thousands of lines) and
Doxygen `@defgroup` definitions but no Sphinx RST guide pages:

| Library | Header | Lines | Description |
|---------|--------|-------|-------------|
| `lib/tapi_job/` | `tapi_job.h` | 2000+ | Agent job control API |
| `lib/tapi_bpf/` | `tapi_bpf.h` | 21K+ | eBPF/XDP testing API |
| `lib/tapi_dpdk/` | `tapi_dpdk.h` | 14K+ | DPDK framework integration |
| `lib/tapi_dns/` | `tapi_dns_unbound.h` | 11K+ | DNS protocol testing |
| `lib/tapi_env/` | `tapi_env.h` | 2000+ | Network environment config |
| `lib/tapi_tool/` | various | | Generic tool wrapper library |
| `lib/tapi_fio/` | `tapi_fio.h` | | FIO testing integration |
| `lib/tapi_gtest/` | `tapi_gtest.h` | | Google Test integration |
| `lib/tapi_nvme/` | `tapi_nvme.h` | | NVMe device testing |
| `lib/tapi_packetdrill/` | `tapi_packetdrill.h` | | Packetdrill test tool |
| `lib/tapi_performance/` | `tapi_performance.h` | | Performance testing |
| `lib/tapi_radius/` | `tapi_radius.h` | | RADIUS protocol testing |
| `lib/tapi_serial/` | `tapi_serial.h` | | Serial console testing |

### 6.2 Missing tool documentation

| Tool | Path | Description |
|------|------|-------------|
| log_server | `tools/log_server/` | Centralized log collection server |
| log_streaming | `tools/log_streaming/` | Real-time log streaming |
| log_parse | `tools/log_parse/` | Log parsing utilities |
| ndn_parser | `tools/ndn_parser/` | NDN format parser |

### 6.3 Missing agent documentation

Agents `proxy`, `power-ctl`, and `switch-ctl` exist in `agents/` but have no
RST documentation pages. Currently only Unix and Windows agents are documented.

### 6.4 Missing infrastructure documentation

| Topic | Location | Description |
|-------|----------|-------------|
| Meson build options | `meson_options.txt` | 53 options, ~45 undocumented in RST |
| YAML configuration | `engine/configurator/` | YAML config format undocumented |
| CI/CD pipelines | `jenkins/` | Jenkins pipelines and helper scripts |
| Script libraries | `scripts/lib*` | Core bash utilities for TE |
| BPF infrastructure | `bpf/` | eBPF programs and TC rules |
| Configuration model | `doc/cm/` | 71 YAML files, no RST index |
| Cross-compilation | `engine/builder/te_cross_build_meson` | Meson cross-compilation guide |
| Apps | `apps/dpdkrpc/`, `apps/ta_rpcprovider/` | Engine applications |

---

## 7. Sphinx conf.py notes

- `doc/sphinx/conf.py` line 25: Copyright year is `2019`, consider updating.
- `doc/sphinx/conf.py` line 29: Release version is `1.0`, may need update.
- Requires `DOXYREST_PREFIX` environment variable to be set (line 16).

---

## Summary

| Category | Count |
|----------|-------|
| ~~Wrong code-block language tags~~ | ~~50+ instances across 10 files~~ DONE |
| Outdated XML-to-YAML references | 2 files |
| Incorrect content (factual errors) | 8 distinct issues |
| ~~Broken references/links~~ | ~~7 instances~~ DONE |
| Typos and spelling errors | 17 instances across 13 files |
| Formatting issues | 4 instances |
| Missing TAPI module pages | 13 libraries |
| Missing tool documentation | 4 tools |
| Missing agent documentation | 3 agents |
| Missing infrastructure docs | 8 topics |
