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

### 1.2 Outdated XML file references (should be YAML) -- DONE

~~Configuration model files were renamed from `.xml` to `.yml`.~~

Both references updated to `cm_base.yml`:
- `group_te_engine_conf.rst`
- `group_te_agents_win.rst`

---

## 2. Incorrect Content (HIGH priority) -- DONE

### 2.1 group_te_agents.rst -- DONE

- ~~Removed incorrect FreeBSD claim~~
- ~~Listed all five agent types (unix, win32, proxy, power-ctl, switch-ctl)~~

### 2.2 group_te_engine_dispatcher.rst -- DONE

- ~~Malformed usage line with spurious `=""` XML-like attributes~~
- ~~Option `force` missing `--` prefix~~
- ~~Options `test-wof` and `test-woc` missing `--` prefix~~
- ~~Incomplete `sniff-snaplen` default value (now "unlimited")~~
- ~~Hardcoded developer path replaced with `${TE_RUN_DIR}/caps`~~

### 2.3 group_te_engine_rcf.rst -- DONE

- ~~Updated config examples from deprecated `confstr="..."` attribute syntax
  to current `<conf name="...">value</conf>` element syntax~~

### 2.4 group_trc.rst -- DONE

- ~~Fixed "Failed, as expected" description: "expected to pass and passed"
  → "expected to fail and failed"~~

### 2.5 group_tapi_tad_main.rst -- DONE (fixed in earlier commit)

- ~~Commented out reference to non-existent HLD document~~

### 2.6 group_tapi_conf.rst -- DONE

- ~~Replaced non-existent `tapi_conf_eth` reference with `tapi_conf_if`~~

### 2.7 group_te_agents_conf.rst -- DONE

- ~~Replaced `strdup()` with `TE_STRDUP()`, removed unnecessary NULL check~~

### 2.8 group_te_agents_unix.rst -- DONE

- ~~Fixed "Egent" typo to "Agent"~~
- ~~Removed non-existent `agents/unix/rpc` directory reference~~
- ~~Added missing conf subdirectories: ovs, process, rule, tc, vm~~

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

## 4. Typos and Spelling Errors -- DONE

All 17 instances fixed across 9 files (2 others were fixed in section 2).

---

## 5. Formatting Issues -- DONE

All 4 issues fixed.

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
| ~~Incorrect content (factual errors)~~ | ~~8 distinct issues~~ DONE |
| ~~Broken references/links~~ | ~~7 instances~~ DONE |
| ~~Typos and spelling errors~~ | ~~17 instances across 13 files~~ DONE |
| ~~Formatting issues~~ | ~~4 instances~~ DONE |
| Missing TAPI module pages | 13 libraries |
| Missing tool documentation | 4 tools |
| Missing agent documentation | 3 agents |
| Missing infrastructure docs | 8 topics |
