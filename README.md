# WAIN System

**A World-Agent Interactive Narrative System for Visual Novels: WAIN**

WAIN is a World-Agent Interactive Narrative System for LLM-driven visual novels. It manages persistent character, world, and narrative state outside the LLM to support coherent long-term interactions. This project is being developed as an individual graduation project.

## Overview

WAIN keeps persistent narrative state outside the LLM and uses it to guide long-term character behavior and responses in visual novels.

- **State:** Character, relationship, emotion, world, and narrative state
- **Behavior:** Memory, Reflection, Planning, and narrative goal control
- **Interaction:** Free-form dialogue with context built from the current state

The [graduation project proposal (PDF)](Docs/WAIN-System-Vault/06_Assets/260923_WAIN_Proposal-Presentation.pdf) outlines the original project concept and scope.

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


## Academic Information

- **Institution:** Konkuk University
- **Project Type:** Individual Graduation Project
- **Project Period:** 2026.09 ~ 2027.08
- **Project Name:** WAIN System (Full Title: A World-Agent Interactive Narrative System for Visual Novels)

## License

TBD
