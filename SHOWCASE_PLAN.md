# MSH Showcase Plan

## Goal
Create a self-running `2-3 minute` video that feels polished for IT students while still landing a clear wow moment.

## Core Message
The viewer should remember four points:
- `MSH` is not a plain school shell project.
- It looks intentional and cinematic on screen.
- It can launch real `.bat` workflows for project apps.
- It supports real source-code workflow inside the terminal.
- It embeds an AI copilot directly in the shell.

## Recommended Recording Flow
Run one of these:

```powershell
msh.exe
demo
```

For auto-capture:

```powershell
powershell -ExecutionPolicy Bypass -File demo_video.ps1
```

## Demo Structure
### 1. Opening Hook
Time: `0:00 - 0:12`
- Capture the startup animation and `MSH LIVE DEMO MODE` reveal.
- Overlay: `A cinematic shell in C`
- Overlay: `Windows workflow + AI inside`
- Edit: slow push-in, fade from black, no hard glitch spam.

### 2. Batch App Launcher
Time: `0:12 - 0:32`
- Commands: `run_googlemap.bat`, `stop_googlemap.bat`
- Purpose: prove the shell can launch and stop a real project app through `.bat` workflows.
- During this section, let the browser preview `http://localhost:8080` for a few seconds, then return to the shell.
- Overlay: `Run real .bat workflows`
- Overlay: `Launch project apps from the shell`
- Edit: zoom into the service URLs, show the web app briefly, then cut back to the shell on the return focus.

### 3. Source Workflow
Time: `0:32 - 1:02`
- Commands: `tree src -d 1`, `search *.c src`, `grep msh src\main.c`, `head src\main.c 5`
- Purpose: show project inspection and code-oriented workflow.
- Overlay: `Inspect project structure`
- Overlay: `Search source files fast`
- Overlay: `Jump straight to the code`
- Edit: zoom on the `search` results and on the `grep` hit.

### 4. Real Shell Flow
Time: `1:02 - 1:20`
- Commands: redirect text into `test_demo.txt`, read it back, append, remove.
- Purpose: prove this is usable shell workflow, not just visual flair.
- Overlay: `Redirect. Read. Update. Clean.`
- Edit: small punch-in on each `cat` output.

### 5. Visual Modes
Time: `1:20 - 1:36`
- Commands: `color ocean`, `dir`, `color sunset`, `color matrix`
- Purpose: give the showcase a visual reset and highlight terminal identity.
- Overlay: `Built to be seen, not just used`
- Edit: beat-sync cuts on theme changes, no flashy transitions beyond a soft zoom.

### 6. AI Reveal
Time: `1:36 - 2:15`
- Commands: two short `ai ...` prompts, `aimode on`, one natural-language prompt, `!pwd`, `aimode off`
- Purpose: this is the climax. It proves the shell can switch between commands and AI-assisted workflow.
- Overlay: `AI inside the terminal`
- Overlay: `Chat naturally`
- Overlay: `Use ! to run shell commands`
- Edit: strongest zooms in the whole video happen here.

### 7. Ending Shot
Time: `2:15 - 2:25`
- Hold the final prompt or let `exit` close the session.
- Overlay: `MSH`
- Overlay: `C shell + cinematic UX + AI workflow`
- Edit: let the music resolve instead of hard-cutting the end.

## Narration / Caption Script
Use these short captions instead of long subtitles:
- `A cinematic shell in C`
- `Launch project apps`
- `Search what matters`
- `Real shell flow`
- `Visuals with purpose`
- `AI inside the shell`
- `Chat + commands together`

## Zoom and Effect Plan
Use zoom only when new information appears:
- startup reveal
- `search *.c src` results
- `grep msh ...` hit
- each theme switch
- AI responses

Recommended effects:
- slow push-in
- soft glow for text overlays
- subtle vignette
- tiny whoosh on major zooms

Avoid:
- aggressive screen shake
- constant glitch overlays
- thick scanlines
- EDM-style transition spam

## Music Direction
Target mood: `modern cyber ambient`
- BPM: `85-105`
- No vocals if possible
- Keep the bass soft enough that terminal sounds still read cleanly

Suggested arc:
- intro: restrained pulse
- source workflow: steady rhythm
- visual modes: slightly brighter layer
- AI reveal: add width and lift
- ending: clean fade-out

## Sound Design
Keep it light:
- soft keyboard taps
- tiny sweep on zoom
- one low impact sound when AI first responds

Do not overdo hacker cliches.

## Editing Rules
- Show fewer features, but let each feature land.
- Cut on command completion, not randomly.
- Do not zoom unless the viewer has somewhere specific to look.
- The AI block is the climax; protect it with cleaner framing and less clutter.
