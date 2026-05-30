# 🔮 Crystallizer Plugin — Build Guide
### For complete beginners on macOS

---

## What you'll install (all FREE)

| Tool | What it does | Download |
|------|-------------|----------|
| **Xcode** | Apple's compiler — turns code into a plugin | Mac App Store |
| **JUCE** | The audio plugin framework the code is built on | juce.com |

---

## Step 1 — Install Xcode

1. Open the **Mac App Store**
2. Search **Xcode** and click **Get / Install**
3. After it installs, open it once and accept the license agreement
4. Open **Terminal** (press ⌘+Space, type "Terminal") and run:
   ```
   xcode-select --install
   ```

---

## Step 2 — Install JUCE

1. Go to **https://juce.com/get-juce/** and download the free version
2. Open the downloaded `.dmg` and drag **JUCE** to your Applications folder
3. Inside the JUCE folder, open **Projucer** (the JUCE project manager app)

---

## Step 3 — Open the project in Projucer

1. In Projucer, choose **File → Open**
2. Navigate to the `Crystallizer` folder you received
3. Open **Crystallizer.jucer**
4. In Projucer, click **File → Save Project and Open in IDE**
   - This opens Xcode automatically

---

## Step 4 — Set the JUCE modules path (first time only)

If Projucer shows a warning about modules:
1. Go to **Projucer → Preferences → Paths**
2. Set **JUCE Modules** to the `modules` folder inside your JUCE installation
   - Usually: `/Applications/JUCE/modules`

---

## Step 5 — Build in Xcode

1. In Xcode, at the top choose the **Release** scheme (not Debug)
   - Click the scheme dropdown next to the ▶ button → pick **Crystallizer - Release**
2. Press **⌘ + B** to build
3. Wait for "Build Succeeded" ✅ in the top bar

---

## Step 6 — Install the plugin

After a successful build, your AU plugin will be at:

```
~/Library/Audio/Plug-Ins/Components/Crystallizer.component
```

Logic Pro X scans this folder automatically.

---

## Step 7 — Load it in Logic Pro X

1. Open **Logic Pro X**
2. Create an **Audio** or **Software Instrument** track
3. Click the **Audio FX** slot on the channel strip
4. Go to **Audio Units → YourName → Crystallizer**
5. 🎉 It will load with 5 knobs:
   - **Grain Size** — length of each reversed grain (20–500 ms)
   - **Pitch** — pitch shift in semitones (–24 to +24)
   - **Feedback** — how much the output feeds back into itself
   - **Spread** — random pitch variation between grains
   - **Mix** — dry/wet blend

---

## Troubleshooting

**"Plugin not showing in Logic"**
- Logic may need to re-scan. Go to **Logic → Preferences → Plug-in Manager** and click **Reset & Rescan**

**"Build failed — module not found"**
- Double-check your JUCE Modules path in Projucer preferences (Step 4)

**"Xcode says 'signing' error"**
- In Xcode, click the project name in the left sidebar → **Signing & Capabilities** → change Team to **None** or your Apple ID

---

## Tips for using the effect

- **Pad/Chord sounds**: Pitch = –12, Feedback = 0.6, Grain Size = 200ms → lush reverse shimmer
- **Glitchy stabs**: Grain Size = 30ms, Spread = 8, Feedback = 0.3 → crystalline chaos
- **Subtle shimmer**: Pitch = +12, Mix = 0.25, Feedback = 0.2 → octave-up ghost layer

---

*Built with JUCE 7. Tested on macOS Ventura / Sonoma with Logic Pro X 10.7+*
