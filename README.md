# WAIN System

**A World-Agent Interactive Narrative System for Visual Novels**

WAIN is an Unreal Engine plugin for LLM-driven visual novels. It maintains persistent narrative state outside the LLM to guide characters and generated responses over long interactions. This individual graduation project is developed at Konkuk University.

## Overview

WAIN keeps persistent narrative state outside the LLM and uses it to guide long-term character behavior and responses in visual novels.

- **State:** Character, relationship, emotion, world, and narrative state
- **Behavior:** Memory, Reflection, Planning, and narrative goal control
- **Interaction:** Free-form dialogue with context built from the current state

The [graduation project proposal (PDF)](Docs/WAIN-Vault/07_Assets/26-2_졸업프로젝트_WAIN_제안서.pdf) outlines the original project concept and scope.

## Project Status

Currently under development.

The initial prototype focuses on a Single-Agent test scenario to validate the core components of the WAIN System before extending the architecture to support multiple Character Agents.

## Tech Stack

- C++
- Unreal Engine
- Unreal Engine Plugin System
- LLM API integration
- JSON-based state and context data
- Git / GitHub

## Repository Scope

This is the main project repository for the **WAIN System**. The core Unreal Engine plugin source is intended to live under `Plugins/WAINSystem/` alongside the system documentation.

The project proposal and engineering notes are kept with the repository. Game-specific content, visual novel assets, and the Unreal testbed project used for integration and evaluation are maintained separately.

### Repository Layout

```text
WAIN-System/
├── Docs/
│   └── WAIN-Vault/        Project docs and proposal under 07_Assets/
├── Plugins/
│   └── WAINSystem/        Core Unreal Engine plugin source
└── README.md
```

## Academic Information

This repository is an individual graduation project developed at **Konkuk University**.

- **Institution:** Konkuk University
- **Project Type:** Individual Graduation Project
- **Project Name:** WAIN System
- **Full Title:** A World-Agent Interactive Narrative System for Visual Novels

## License

TBD
