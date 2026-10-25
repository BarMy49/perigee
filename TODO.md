# Project Perigee — Productivity-First GameStream Client for Linux

## 1. Your Role

Act as a senior C++/Qt developer, Linux desktop engineer, software architect, and GameStream protocol specialist.

You are responsible for analyzing, designing, and eventually developing **Perigee**, a modern open-source Moonlight Qt fork optimized for remote desktop productivity while retaining excellent gaming performance.

This project will use an AI-assisted development workflow with separate planning and implementation stages.

**Do not start implementing application features immediately. Your first task is a comprehensive technical analysis, architecture proposal, and development plan.**

Base all conclusions on actual source code, repository history, documentation, and verifiable evidence.

Never invent APIs, features, file paths, or implementation details.

## 2. Project Identity

The project is officially named **Perigee**.

The name comes from orbital mechanics, where perigee is the point in an orbit closest to Earth. It represents the project's objective: making a remote computer feel as close and responsive as a local machine.

### Project Information

- **Name:** Perigee
- **Repository name:** `perigee`
- **Planned executable:** `perigee`
- **Primary platform:** Linux, initially Fedora
- **Primary language:** C++
- **UI framework:** Qt/QML
- **Foundation:** Moonlight Qt
- **Primary host:** Apollo
- **Additional compatibility:** Standard Sunshine servers
- **Purpose:** Productivity-first, low-latency remote desktop and game streaming

### Tagline

*Your remote desktop, closer than ever.*

### Project Description

Perigee is an open-source, low-latency remote desktop and game streaming client for Linux, built on Moonlight Qt with enhanced Apollo integration.

Designed for productivity, engineered for performance.

### Branding and Compatibility Requirements

The project is named Perigee, but it is based on Moonlight Qt.

**Preserve existing upstream identifiers where necessary for compatibility, but plan a clean migration of application branding, executable names, configuration paths, and desktop integration.**

Investigate the implications of renaming:

- Application name and window titles.
- Executable and package names.
- Qt application and organization identifiers.
- Application configuration paths.
- Persistent settings and pairing data.
- Application icons and visual assets.
- Linux desktop entry identifiers.
- AppStream metadata.
- Flatpak application identifiers.
- RPM package identifiers.
- Logging and diagnostic identifiers.
- Build output names.
- Internal namespaces and class names.
- GameStream client identifiers.
- Host pairing and authentication identifiers.
- Protocol-specific identifiers.

Determine which identifiers are purely cosmetic and which participate in protocol communication, authentication, configuration, or compatibility.

Do not blindly rename internal symbols.

Avoid mass search-and-replace operations across the codebase.

Prefer an incremental, compatibility-aware migration that minimizes upstream divergence.

Perigee should have its own independent user configuration where appropriate, without unintentionally overwriting Moonlight Qt settings or losing existing host pairing information.

Investigate whether existing Moonlight settings can be imported through an explicit migration mechanism.

Document any protocol identifiers that must remain unchanged.

Ensure branding changes do not compromise compatibility with Apollo, Sunshine, or upstream Moonlight components.

**During this planning session, design the branding migration but do not implement it.**

## 3. Project Background

I currently use Apollo as my streaming host on Windows and Moonlight Qt as my client on Fedora Linux.

Apollo:
https://github.com/ClassicOldSong/Apollo

I primarily use this setup for:

- Software development and programming.
- Remote Windows desktop access.
- Running JetBrains IDEs and other development tools.
- General office work and productivity.
- Occasional gaming.

I particularly appreciate the low latency, high image quality, hardware-accelerated video streaming, and relatively low GPU overhead of the GameStream protocol.

I want to preserve these characteristics rather than replace GameStream with another remote desktop protocol.

However, existing clients have limitations:

- **Moonlight Qt:** Reliable and fast but lacks convenient integration with several Apollo-specific features.
- **Artemis Qt:** Contains useful Apollo integrations but appears poorly maintained, lacks a dependable stable Linux release, and does not work on my Fedora installation.
- **Vibemis:** Contains potentially useful improvements, but it crashes on my system, and I dislike its handheld-oriented interface.

My goal is to develop Perigee by forking upstream Moonlight Qt and selectively integrating relevant features from other projects.

Perigee must remain compatible with standard Sunshine servers, not only Apollo.

## 4. Reference Repositories

Analyze the following repositories.

### Primary Codebase — Moonlight Qt

https://github.com/moonlight-stream/moonlight-qt

This will be the foundation of Perigee.

Inspect its architecture, build system, dependencies, QML interface, platform integrations, hardware decoding, input management, and streaming implementation.

### Apollo — Host Implementation

https://github.com/ClassicOldSong/Apollo

Study Apollo's architecture, GameStream extensions, client-specific functionality, host APIs, permission system, virtual displays, and clipboard integration.

Determine which Apollo capabilities require explicit client support and which already work with standard Moonlight.

### Artemis Qt — Apollo Integration Reference

https://github.com/wjbeckett/artemis

Identify Apollo-specific modifications and determine whether they can be ported into a modern Moonlight Qt codebase.

Do not assume its implementation is correct, current, or compatible with upstream.

### Vibemis — Alternative Implementation Reference

https://github.com/navyas321/vibemis

Inspect its Apollo-specific functionality, relevant fixes, architectural changes, and differences from Artemis Qt.

Use it as a technical reference rather than as a foundation for Perigee.

Determine whether useful modifications can be isolated from its UI and platform-specific changes.

### Moonlight Common C — Protocol Implementation

https://github.com/moonlight-stream/moonlight-common-c

Study the core GameStream protocol implementation and its relationship with Moonlight Qt.

Identify which features can be implemented entirely in the Qt client and which require modifications to the underlying protocol library.

Avoid modifying the streaming core unless demonstrably necessary.

### Repository Inspection Requirements

1. Obtain and inspect the latest available source code for each repository.
2. Record the exact branch and commit SHA used for each reference.
3. Identify relevant upstream commits, forks, and dependency relationships.
4. Compare existing implementations before proposing new code.
5. Cite exact source files, functions, classes, and relevant code locations.
6. Include clickable GitHub links to important source-code references where possible.
7. Identify unavailable repositories, inaccessible files, or incomplete evidence.
8. Never claim that a feature exists without verifying its implementation.
9. Consider the licensing implications of reusing code from each repository.
10. Keep reference repositories separate from the primary working tree.

## 5. Product Vision

Perigee should feel like a modern Linux remote workstation client rather than a gaming launcher.

It should combine:

- Moonlight's efficient and reliable streaming.
- Apollo's advanced remote desktop functionality.
- A clean, professional desktop interface.
- Strong Fedora and Wayland compatibility.
- Convenient productivity tools.
- Full keyboard and mouse functionality.
- Optional gaming features without making gaming the primary focus.

The initial target is Fedora Linux, but the architecture should avoid unnecessary Fedora-specific assumptions so other Linux distributions can eventually be supported.

Perigee should have its own recognizable visual identity while preserving attribution to Moonlight Qt and other upstream projects.

## 6. Planned Features

Investigate the feasibility and implementation strategy for each feature.

### A. Apollo Integration

**Priority: High**

- Detect whether the connected host uses Apollo or standard Sunshine.
- Support Apollo-specific capabilities without breaking ordinary Sunshine connections.
- Integrate Apollo virtual-display management.
- Support host display resolution and refresh-rate configuration.
- Implement bidirectional clipboard synchronization.
- Expose relevant Apollo permission information and controls.
- Integrate supported host-side commands.
- Support Apollo-specific application launching where available.

Determine which operations use GameStream protocol extensions, which use separate APIs, and which require authentication or additional permissions.

Never assume that Apollo automatically exposes a feature through GameStream merely because the feature exists on the host.

### B. Productivity Features

**Priority: High**

- Reliable Unicode text input.
- Bidirectional text clipboard synchronization.
- Image clipboard synchronization if technically feasible.
- Configurable keyboard shortcut forwarding.
- Correct handling of Windows shortcuts from Fedora.
- Convenient switching between local and remote applications.
- Proper mouse pointer and cursor integration.
- Seamless windowed and fullscreen operation.
- Convenient remote desktop controls without disconnecting.
- File transfer support.

For file transfers, investigate whether a suitable existing mechanism can be reused.

Consider optional SFTP or another secure transfer mechanism if Apollo does not expose a suitable native API.

Avoid introducing insecure or unnecessary host-side services.

### C. Streaming Quality and Performance

**Priority: Critical**

Preserve the existing Moonlight streaming pipeline wherever possible.

Investigate:

- H.264, HEVC, and AV1 compatibility.
- NVIDIA NVENC compatibility on the host.
- VA-API hardware decoding on Linux.
- Relevant Vulkan decoding or presentation paths.
- HDR capabilities.
- High-refresh-rate displays.
- Variable refresh rate support.
- Frame pacing.
- Input latency.
- Network jitter handling.
- YUV 4:4:4 support and hardware limitations.
- Multimonitor and dynamic resolution behavior.

Determine whether YUV 4:4:4 would significantly benefit programming workloads involving fine text and syntax highlighting.

Do not sacrifice hardware acceleration or introduce unnecessary CPU/GPU overhead for UI enhancements.

**Preserving upstream streaming behavior is more important than implementing every proposed feature.**

### D. Desktop-Focused User Interface

**Priority: High**

The UI should use Qt/QML and remain reasonably consistent with modern Linux desktop conventions.

Desired characteristics:

- Minimalist design.
- Clean host and application selection.
- Easy access to remote desktops.
- No unnecessary gaming-focused interface elements.
- Convenient configuration.
- Native-feeling window decorations.
- Responsive layouts.
- Light and dark themes.
- Good keyboard navigation.
- Consistent typography and spacing.
- Optional system tray integration, where supported.
- Consistent Perigee branding.

Investigate whether the current Moonlight Qt interface can be incrementally modernized rather than rewritten.

Avoid introducing large UI dependencies unless they provide a clear benefit.

Do not design or implement a complete visual overhaul during the initial planning phase.

### E. In-Stream Controls

Investigate implementing an unobtrusive overlay or toolbar providing:

- Disconnect and reconnect.
- Fullscreen/windowed switching.
- Mouse capture controls.
- Clipboard synchronization controls.
- Display resolution adjustments.
- Host display selection.
- Streaming statistics.
- Session settings.
- Remote commands where supported.

The interface must remain accessible during desktop use without interfering with games or keyboard shortcuts.

### F. Separate Productivity and Gaming Profiles

Investigate configurable streaming profiles.

**Programming Profile:**

- High text clarity.
- Reliable input.
- Clipboard synchronization.
- Sensible frame rate and bitrate.
- Desktop-friendly mouse behavior.

**Gaming Profile:**

- Minimum latency.
- High frame rate.
- Controller support.
- HDR and VRR where available.
- Appropriate bitrate and codec settings.

Profiles should reuse existing Moonlight configuration functionality where practical.

### G. Linux Platform Integration

**Priority: High**

Investigate:

- Fedora GNOME on Wayland.
- Fedora KDE Plasma on Wayland.
- X11/XWayland compatibility.
- Wayland clipboard limitations.
- Input capture and shortcut restrictions.
- XDG desktop integration.
- Portals where appropriate.
- PipeWire relevance.
- RPM packaging.
- Flatpak packaging.
- AppImage packaging.
- Hardware decoder dependencies.

Do not assume functionality working on X11 will automatically work on Wayland.

Identify which limitations can be solved inside the client and which are imposed by the compositor or security model.

## 7. Architecture Requirements

Perigee must remain maintainable.

Prefer the following architecture:

1. Keep upstream Moonlight Qt as the foundation.
2. Isolate Apollo-specific behavior into clearly defined components.
3. Preserve standard Sunshine compatibility.
4. Avoid unnecessary changes to Moonlight Common C.
5. Separate protocol communication, platform integration, application logic, and UI responsibilities.
6. Reuse existing Qt abstractions where appropriate.
7. Avoid unnecessary dependencies and frameworks.
8. Follow existing upstream coding conventions.
9. Preserve the existing build system unless a change is technically justified.
10. Make future upstream merges as straightforward as possible.
11. Isolate Perigee-specific branding and application metadata where practical.
12. Preserve protocol-critical upstream identifiers.
13. Avoid renaming internal classes, namespaces, or interfaces solely for branding.
14. Keep existing authentication and pairing behavior compatible.
15. Ensure Perigee can coexist with an installed Moonlight Qt application.

Investigate whether Apollo functionality should be implemented as a dedicated integration layer with runtime capability detection.

Compare that approach against directly modifying existing Moonlight classes.

Consider appropriate use of interfaces, composition, Qt signals/slots, and asynchronous operations.

Avoid overengineering.

## 8. Required Technical Analysis

Perform the following analyses before proposing implementation.

### Analysis 1 — Moonlight Qt Architecture

Explain:

- Main application entry points.
- Important C++ classes.
- Qt/QML structure.
- Streaming session lifecycle.
- Host discovery and pairing.
- GameStream communication.
- Keyboard and mouse handling.
- Clipboard handling, if any.
- Linux platform-specific components.
- Settings and configuration persistence.
- Build system and dependencies.
- Testing infrastructure.

Provide a logical architecture diagram using Mermaid.

### Analysis 2 — Apollo Protocol and Client Extensions

Identify exactly how Apollo extends standard Sunshine behavior.

For every relevant feature, document:

- Protocol or API used.
- Host-side implementation.
- Required client-side implementation.
- Authentication and permission requirements.
- Whether upstream Moonlight Qt already supports it.
- Whether Artemis Qt implements it.
- Whether Vibemis implements it.
- Compatibility risks.

Distinguish implemented functionality from documentation, proposals, and assumptions.

### Analysis 3 — Fork Comparison

Compare Moonlight Qt, Artemis Qt, and Vibemis.

Identify:

- Common code ancestry.
- Relevant commits.
- Major architectural differences.
- Apollo-specific code.
- Changes to streaming behavior.
- Linux platform-specific changes.
- Potential regressions.
- Outdated dependencies.
- Useful bug fixes.
- Features that should not be ported.

For each reusable feature, identify its exact implementation and preferred source.

### Analysis 4 — Fedora Build and Packaging

Determine the correct way to build the current Moonlight Qt version on Fedora.

Provide:

- Required development packages.
- Exact Fedora installation commands.
- Build commands.
- Submodule requirements.
- Runtime dependencies.
- Debug build instructions.
- Relevant environment variables.
- Debugging tools.
- Packaging options.

Verify these instructions against the inspected build configuration.

Do not assume old instructions still apply.

Determine how to install and run Perigee without overwriting the existing Moonlight installation or corrupting its settings and pairing data.

### Analysis 5 — Risks and Feasibility

Identify significant technical risks, including:

- Apollo API instability.
- Compatibility with future Moonlight updates.
- Protocol/library version conflicts.
- Qt version compatibility.
- Fedora Wayland restrictions.
- Input and clipboard security.
- Hardware decoding regressions.
- Licensing obligations.
- Difficulty integrating old fork implementations.
- Application identifier collisions.
- Configuration migration errors.
- Pairing or authentication regressions caused by branding changes.

Assess each risk and propose mitigation measures.

Review GPL obligations, attribution, and license compatibility before recommending source-code reuse.

### Analysis 6 — Perigee Branding Migration

Determine exactly how Perigee can obtain an independent application identity while preserving upstream compatibility.

Inspect the current Moonlight Qt implementation for:

- Executable naming.
- Qt application metadata.
- Organization and application settings identifiers.
- Configuration file locations.
- Persistent pairing information.
- Desktop integration.
- Icon resources.
- Build scripts.
- Packaging metadata.
- Protocol-level client identification.
- Any hardcoded references to Moonlight.

Classify each identifier as:

1. Safe to rename.
2. Renameable with compatibility migration.
3. Required to remain unchanged.
4. Requires additional investigation.

Propose an incremental migration strategy.

Include a plan for maintaining clean separation between Perigee and the existing Moonlight installation.

Do not perform the migration during planning.

## 9. AI-Assisted Development Workflow

This project uses a deliberate two-stage development strategy.

The objective is to reserve expensive reasoning for difficult architectural decisions while using more economical reasoning for clearly specified implementation tasks.

### Stage A — Planning and Architecture

This stage is responsible for:

- Deep repository analysis.
- Understanding source-code architecture.
- Comparing upstream and downstream implementations.
- Identifying Apollo protocol extensions.
- Choosing architectural boundaries.
- Investigating performance and compatibility risks.
- Designing a maintainable integration strategy.
- Planning Perigee branding and identifier migration.
- Producing a detailed implementation roadmap.
- Establishing coding and testing rules.
- Creating persistent project documentation.

Use extensive reasoning to examine competing architectural approaches.

Consider trade-offs explicitly.

Do not make architectural choices solely because they are easier to implement.

Look for opportunities to reuse proven upstream implementations.

### Stage B — Implementation

This stage begins only after the planning phase has produced sufficient documentation and the implementation plan has been approved.

Sessions should:

1. Read `AGENTS.md` and the relevant architecture documents.
2. Identify the current milestone.
3. Inspect the relevant source files.
4. Implement one small, well-defined task.
5. Follow the approved architectural design.
6. Add or update appropriate tests.
7. Run available build and verification commands.
8. Report changes, test results, and outstanding issues.
9. Update project documentation when necessary.
10. Stop after completing the assigned task.

Do not redesign the project architecture during routine implementation.

If a task reveals an architectural issue, document the problem and propose a change before proceeding.

### Managing Context and Token Usage

Optimize the workflow for efficient use of the available context.

- Avoid repeatedly analyzing the entire repository.
- Preserve architectural conclusions in project documentation.
- Read only relevant documents and source files for routine tasks.
- Do not unnecessarily include entire large source files in responses.
- Reference exact file paths and symbols instead.
- Keep implementation tasks narrowly scoped.
- Avoid repeating analyses that have already been documented.
- Revisit architectural decisions only when new evidence justifies it.
- Never sacrifice correctness merely to reduce token consumption.

The objective is to make Medium-effort sessions effective without requiring them to rediscover decisions made during Max-effort planning.

## 10. Persistent Project Documentation

**Critical requirement**

Before implementation begins, preserve the architectural research and development plan as persistent Markdown files inside the Perigee repository.

Do not rely on chat history as the only source of architectural decisions.

The documentation must be sufficient for another coding agent to continue development without access to the original planning conversation.

### Required Documentation Structure

Use the following structure as a starting point, adapting it only when the existing repository layout justifies a different location.

```text
perigee/
├── AGENTS.md
├── docs/
│   ├── ARCHITECTURE.md
│   ├── APOLLO_PROTOCOL.md
│   ├── REPOSITORY_ANALYSIS.md
│   ├── IMPLEMENTATION_PLAN.md
│   ├── DEVELOPMENT_RULES.md
│   ├── FEDORA_BUILD.md
│   ├── BRANDING.md
│   └── DECISIONS.md
├── ...
```

### AGENTS.md

Create an `AGENTS.md` file at the repository root.

This file should serve as the primary entry point for future AI coding sessions.

Include:

- Project name and purpose.
- Primary target platform.
- Supported host types.
- Technology stack.
- Key architectural boundaries.
- Important directories and files.
- Relevant documentation references.
- Build and testing commands.
- Coding conventions.
- Apollo compatibility requirements.
- Sunshine compatibility requirements.
- Performance preservation rules.
- Perigee branding and identifier preservation rules.
- Git workflow expectations.
- Security and licensing requirements.
- Common pitfalls.
- Rules for modifying the streaming pipeline.
- Instructions for reporting test results.

Keep `AGENTS.md` concise enough to be useful as recurring agent context.

Move detailed explanations into the appropriate files under `docs/`.

Do not duplicate entire architecture documents inside `AGENTS.md`.

### docs/ARCHITECTURE.md

Document:

- Current Moonlight Qt architecture.
- Important components and responsibilities.
- Dependency relationships.
- Streaming lifecycle.
- Host communication.
- Qt/QML integration.
- Platform-specific behavior.
- Proposed Apollo integration architecture.
- Design boundaries.
- Mermaid diagrams.

Clearly distinguish existing architecture from proposed architecture.

### docs/APOLLO_PROTOCOL.md

Document:

- Verified Apollo protocol extensions.
- Relevant host APIs.
- Authentication and authorization.
- Clipboard communication.
- Virtual-display management.
- Host commands.
- Protocol compatibility.
- Known limitations.
- Exact source-code references.

Clearly mark unverified behavior.

Do not invent protocol details.

### docs/REPOSITORY_ANALYSIS.md

Document:

- All inspected repositories.
- Repository URLs.
- Branches and commit SHAs.
- Important source files.
- Relevant code differences.
- Reusable implementations.
- Compatibility risks.
- Licensing observations.
- Recommended source of each feature.

Include links to specific commits and source-code locations where practical.

### docs/IMPLEMENTATION_PLAN.md

Create a milestone-based implementation roadmap.

For each milestone, include:

- Goal.
- Scope.
- Technical dependencies.
- Relevant source files.
- Proposed implementation.
- Expected behavior.
- Testing strategy.
- Acceptance criteria.
- Potential blockers.
- Completion status.

Break large features into independently testable tasks.

Use Markdown checklists for actionable work.

Do not mark a milestone complete without verifying its acceptance criteria.

### docs/DEVELOPMENT_RULES.md

Define:

- C++ coding conventions.
- Qt/QML conventions.
- Error handling principles.
- Asynchronous operation guidelines.
- Thread-safety expectations.
- Security requirements.
- Dependency management.
- Performance constraints.
- Testing expectations.
- Documentation update rules.
- Rules for AI-generated changes.
- Review requirements.

Prefer existing upstream conventions over inventing an incompatible style guide.

### docs/FEDORA_BUILD.md

Document:

- Supported Fedora development environment.
- Development dependencies.
- Build commands.
- Debugging commands.
- Runtime requirements.
- Hardware decoding dependencies.
- Wayland/X11 considerations.
- Packaging recommendations.
- Known build issues.

Distinguish verified instructions from proposed or untested commands.

### docs/BRANDING.md

Create a dedicated Perigee branding and identity migration plan.

Document:

- Official application name.
- Project description and tagline.
- Planned executable name.
- Planned package naming.
- Desktop entry identification.
- Qt application metadata.
- Settings and configuration directory strategy.
- Pairing information migration strategy.
- Icon and application asset requirements.
- License and upstream attribution requirements.
- Identifiers that must remain unchanged.
- Identifiers safe to rename.
- Compatibility risks.
- Incremental migration tasks.
- Validation and regression tests.

Recommend an appropriate reverse-DNS application identifier, but do not assume a GitHub username or organization identifier that has not been provided.

Clearly separate branding changes from protocol-level identifiers.

Do not create or replace application icons during planning.

### docs/DECISIONS.md

Maintain an architecture decision log.

Each significant decision should include:

- Decision identifier.
- Date.
- Context.
- Alternatives considered.
- Selected approach.
- Technical rationale.
- Consequences.
- Relevant references.
- Status.

The goal is to prevent future agents from accidentally reversing deliberate architectural decisions.

### Documentation Quality Requirements

All documentation should be:

- Technically precise.
- Consistent with inspected source code.
- Easy for both humans and coding agents to navigate.
- Free from unsupported assumptions.
- Updated when implementation changes the documented design.

Use links between documents where useful.

Avoid redundant explanations.

Documentation is part of the project, not an optional afterthought.

## 11. Development Strategy

Create an incremental implementation roadmap.

### Phase 0 — Baseline

Build current upstream Moonlight Qt on Fedora.

Confirm that an unmodified build can connect to Apollo and that streaming, audio, hardware decoding, mouse input, and keyboard input work.

Establish a reproducible development environment.

Record baseline behavior and performance characteristics.

### Phase 1 — Perigee Identity and Application Isolation

Implement the minimum branding and application identity changes required to distinguish Perigee from Moonlight Qt.

Focus on:

- Application metadata.
- Executable naming.
- Configuration isolation.
- Desktop integration.
- Preserving protocol compatibility.
- Preserving a clean upstream merge path.

Avoid unnecessary internal renaming.

Ensure Perigee and Moonlight can coexist.

### Phase 2 — Apollo Foundation

Implement Apollo host detection and the minimum infrastructure needed for isolated Apollo-specific capabilities.

Preserve standard Sunshine behavior.

### Phase 3 — Clipboard Integration

Implement secure bidirectional text clipboard synchronization.

Handle permissions, synchronization loops, reconnects, and Linux clipboard restrictions.

Add automated tests where feasible.

### Phase 4 — Display Management

Add Apollo virtual-display controls and relevant resolution/refresh-rate configuration.

### Phase 5 — Productivity UI

Improve navigation, desktop configuration, keyboard handling, and in-stream controls.

Introduce consistent Perigee branding throughout the interface.

### Phase 6 — Additional Integrations

Investigate file transfer, expanded clipboard support, configurable profiles, and additional Apollo commands.

### Phase 7 — Distribution and Maintenance

Set up automated builds and releases.

Investigate RPM, Flatpak, and AppImage distribution.

Establish a practical strategy for merging future upstream Moonlight changes.

You may recommend changing this order if source-code analysis reveals technical dependencies.

## 12. Testing Requirements

Propose an appropriate testing strategy covering:

- Standard Sunshine compatibility.
- Apollo compatibility.
- Host pairing and authentication.
- Clipboard synchronization.
- Virtual-display management.
- Keyboard and mouse input.
- Fedora GNOME Wayland.
- Fedora KDE Wayland.
- X11 where supported.
- Hardware video decoding.
- Latency and frame pacing.
- High-resolution displays.
- Reconnection and network interruptions.
- Memory leaks and resource usage.
- Regression testing following upstream merges.
- Perigee and Moonlight installation coexistence.
- Separate configuration storage.
- Existing Moonlight configuration migration.
- Application identifier consistency.
- Packaging and desktop integration.

Use automated testing where possible.

Identify functionality requiring physical GPU hardware or real host/client integration.

**Distinguish tests actually executed from tests merely proposed.**

Never claim successful compilation or runtime testing unless those operations were performed and verified.

## 13. Rules for AI-Generated Code

Follow these rules during implementation:

1. Inspect existing code before proposing modifications.
2. Reuse verified implementations when appropriate.
3. Avoid generating duplicate functionality.
4. Preserve compatibility with upstream.
5. Do not rewrite unrelated components.
6. Keep modifications small and understandable.
7. Prefer one feature per implementation branch.
8. Explain architectural decisions.
9. Identify relevant source files before editing.
10. Add appropriate tests.
11. Run available compilation and static checks after modifications.
12. Never report a successful build without verification.
13. Clearly report untested functionality.
14. Document new dependencies and configuration changes.
15. Do not perform broad refactoring merely to simplify generated code.
16. Do not introduce performance regressions without explicit justification.
17. Do not commit, push, or publish changes without authorization.
18. Read `AGENTS.md` before beginning a new implementation task.
19. Follow the approved implementation roadmap.
20. Update relevant documentation when functionality changes.
21. Do not silently deviate from architectural decisions.
22. Avoid unnecessary repository-wide searches when the relevant locations are already documented.
23. Do not blindly rename Moonlight-specific identifiers.
24. Preserve protocol-level compatibility during branding changes.
25. Avoid overwriting existing Moonlight configuration or pairing data.
26. Keep Perigee-specific modifications isolated wherever practical.

If a proposed implementation could interfere with the streaming pipeline, analyze the consequences before modifying it.

## 14. Development Session Protocol

Each implementation session should follow a consistent workflow.

### At the Beginning

1. Read `AGENTS.md`.
2. Read the relevant sections of `docs/IMPLEMENTATION_PLAN.md`.
3. Inspect relevant architecture and protocol documentation.
4. Identify the assigned milestone.
5. Check the repository's current state.
6. Identify the files likely to require modifications.

### Before Editing

Briefly summarize:

- What will be implemented.
- Why the change is necessary.
- Which files will be modified.
- Which architectural constraints apply.
- How the result will be tested.

For a routine task that follows an approved plan, proceed without unnecessary additional approval.

For a significant architectural change, stop and request approval.

### During Implementation

- Make focused changes.
- Follow existing conventions.
- Avoid unrelated refactoring.
- Preserve compatibility.
- Add appropriate tests.
- Investigate unexpected failures rather than ignoring them.

### After Implementation

Report:

- Files changed.
- Behavior implemented.
- Tests executed.
- Build results.
- Remaining limitations.
- Potential regressions.
- Documentation updated.
- Recommended next task.

Update the implementation plan to reflect verified progress.

Never mark an untested feature as fully complete.

## 15. Deliverables for the Initial Planning Task

Your first task must produce a comprehensive technical report.

### A. Executive Summary

Explain the overall feasibility of Perigee and identify significant technical challenges.

### B. Architecture Analysis

Document Moonlight Qt's structure, dependencies, and integration points.

Include a Mermaid architecture diagram.

### C. Repository Comparison

Compare Moonlight Qt, Artemis Qt, Vibemis, Apollo, and Moonlight Common C.

Cite relevant source-code locations and commit SHAs.

### D. Feature Implementation Matrix

Produce a table with:

| Feature | Existing implementation | Required changes | Difficulty | Risk | Priority |
|---|---|---|---|---|---|

Include the major proposed features.

Use evidence-based assessments rather than arbitrary estimates.

### E. Proposed Architecture

Present a modular Apollo integration design.

Explain how it preserves Sunshine compatibility and minimizes upstream conflicts.

Include a Mermaid architecture diagram.

### F. Development Roadmap

Break implementation into incremental milestones.

For each milestone, include:

- Goal.
- Relevant source files.
- Dependencies.
- Implementation approach.
- Tests.
- Acceptance criteria.
- Potential blockers.

### G. Fedora Development Environment

Provide verified setup, build, run, and debugging instructions.

### H. Technical Recommendations

Identify:

- Which Artemis Qt changes are worth porting.
- Which Vibemis changes are worth porting.
- Which features should be reimplemented.
- Which features are unnecessarily risky.
- Which architectural approach provides the best long-term maintainability.

### I. Perigee Branding Strategy

Provide a complete branding and migration proposal.

Identify:

- Required naming changes.
- Application metadata changes.
- Settings and configuration migration.
- Protocol-critical identifiers that must be preserved.
- Desktop integration requirements.
- Packaging requirements.
- Potential compatibility risks.
- Appropriate testing procedures.

Do not modify the application during planning.

### J. First Implementation Milestone

Propose the smallest useful first coding task.

Specify the expected behavior, relevant files, tests, and completion criteria.

### K. Persistent Documentation

Create the project documentation described in Section 10.

Ensure that the documents collectively contain enough information for a separate coding agent to continue development.

### L. Planning-to-Implementation Handoff

Prepare a concise handoff for a future Medium-effort coding session.

The handoff must contain:

1. Current project state.
2. Completed planning work.
3. Approved or proposed architectural decisions.
4. Relevant documentation paths.
5. The first implementation task.
6. Precise acceptance criteria.
7. Required build and test commands.
8. Known limitations and unresolved questions.

Provide a ready-to-use prompt for the first implementation session.

The new agent should not need to read the original planning conversation.

## 16. Completion Criteria for Planning

Planning is complete only when:

- The relevant repositories have been inspected, or access limitations have been documented.
- Important architectural components have been identified.
- Apollo-specific integration points have been investigated.
- Major compatibility and performance risks have been documented.
- A maintainable architectural approach has been proposed.
- Perigee's branding migration has been analyzed.
- An incremental development roadmap exists.
- Persistent Markdown documentation has been created.
- `AGENTS.md` accurately describes the project and development constraints.
- The first implementation task has clear acceptance criteria.
- Uncertainties and unverified assumptions have been explicitly identified.
- A handoff for a Medium-effort coding session has been prepared.

If any requirement cannot be completed, explain the blocker and record the remaining work.

Do not fabricate missing evidence merely to satisfy the checklist.

## 17. Important Constraints for This Initial Session

**THIS IS A PLANNING AND DOCUMENTATION TASK, NOT AN APPLICATION IMPLEMENTATION TASK.**

You are allowed to:

- Inspect source code.
- Clone or fetch reference repositories into separate directories.
- Analyze Git history.
- Inspect dependencies.
- Run non-destructive diagnostic commands.
- Create and modify planning documentation.
- Create `AGENTS.md`.
- Create the requested Markdown files under `docs/`.
- Propose architecture and implementation plans.
- Plan Perigee branding and configuration migration.

You are not allowed to:

- Modify application source code.
- Implement features.
- Rename executables or internal identifiers.
- Replace dependencies.
- Rewrite the UI.
- Change existing build configurations.
- Modify the streaming engine.
- Create commits without authorization.
- Push changes.
- Begin implementation before the planning phase is approved.

Do not assume repository capabilities without verification.

Prioritize:

1. Correctness.
2. Source-code evidence.
3. Streaming performance preservation.
4. Maintainability.
5. Apollo and Sunshine compatibility.
6. Developer productivity.
7. Implementation efficiency.
8. Clean application identity and branding.

The final result should provide a sufficiently detailed technical foundation for another developer or AI coding agent to implement Perigee reliably.

## 18. Final Instructions

Begin by inspecting the reference repositories and mapping the architecture.

Proceed through the required analyses.

Present the findings and proposed architecture.

Plan the Perigee branding migration.

Create the persistent planning documentation.

Prepare a detailed implementation roadmap and first-task handoff.

**Do not begin implementing application functionality in this session.**

Once the analysis and documentation are complete, stop and present the results for review.

The next session will use Medium reasoning effort and follow the approved documentation.

**The objective is not simply to build another Moonlight fork. It is to create Perigee: a maintainable, high-performance, productivity-first Linux client that preserves the strengths of Moonlight while integrating Apollo's advanced remote desktop capabilities.**