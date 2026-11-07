# Vesta Skin Loadout Manager

A standalone Windows utility for organizing cosmetic loadout ideas. It does **not**
connect to Counter-Strike 2, read or write game memory, inject code, or interact
with anti-cheat software. It only stores local profile files.

## Features

- Organize weapon, knife, glove, and other cosmetic ideas by category.
- Store an item name, Wear/Float value (0.00–1.00), and a StatTrak flag.
- Add and remove entries from the current loadout.
- Save and load profiles as portable \`.vsl\` files.

## Build

### GitHub Actions

Push a change to this module on the \`feature/standalone-skin-loadout-manager\`
branch, or run **Build Skin Loadout Manager** manually from the Actions tab.
Download the \`VestaSkinLoadoutManager-windows-x64\` artifact from the completed run.

### Local build (Visual Studio Developer Command Prompt, x64)

\`\`\`bat
cl /nologo /std:c++20 /EHsc /O2 /DUNICODE /D_UNICODE main.cpp /link /SUBSYSTEM:WINDOWS comctl32.lib comdlg32.lib /OUT:VestaSkinLoadoutManager.exe
\`\`\`

This tool is independent of the Vesta runtime and does not apply cosmetic changes
inside the game.
