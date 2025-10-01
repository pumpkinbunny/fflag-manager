# Roblox FFlag Manager

A simple and fast tool for managing Roblox Fast Flags (FFlags) on the PC client. This tool reads a local `fflags.json` file and applies the specified overrides to the running Roblox process at runtime.

![Language](https://img.shields.io/badge/language-C%2B%2B-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Features

- **Dynamic FFlag Overrides**: Modify FFlags in the live Roblox client. Can be run before or during a game session.
- **Configuration via JSON**: Easily manage all your flag overrides in a simple `fflags.json` file.
- **FFlag Cleanup**: Automatically detects FFlags in your JSON that no longer exist in the client and offers to remove them.

## Usage

1.  Place `odessa.exe` and your `fflags.json` file in the same folder.
2.  Launch Roblox.
3.  Run `odessa.exe`. The program will find the Roblox process, apply the FFlags from your JSON file, and report its progress.

### Example `fflags.json`

```json
{
    "DFIntTaskSchedulerTargetFps": "29383",
    "FFlagGameBasicSettingsFramerateCap5": "False",
    "FFlagTaskSchedulerLimitTargetFpsTo2402": "False"
}
```

### Bloxstrap Integration (Recommended)

You can use [Bloxstrap](https://github.com/pizzaboxer/bloxstrap) to automatically run the FFlag manager every time Roblox starts.

1.  Open the Bloxstrap menu and go to the **Custom Integrations** tab.
2.  Click the **+ New** button to add a new integration.
3.  Set the "Application Location" to the path of your `odessa.exe`.

![Bloxstrap Integration Example](https://files.catbox.moe/kkmy36.png)

## Notes & Troubleshooting

-   If the tool reports that it failed to find a FFlag, it most likely means that FFlag is no longer used by the Roblox client and can be removed from your JSON file.
-   Not every FFlag is supported. Some FFlags have unregistered or unavailable "get/set" methods within the client, which makes them impossible to modify.

## Building Notes

This project is built using the latest preview features of C++ with the MSVC compiler. The provided executable is compiled for **Release x64**. The project is not configured for Debug builds.

## Contributing & Issues

This project was made in about one night, so it was pretty rushed. If you find any issues or have suggestions for improvements, feel free to **open an issue** or **submit a pull request**. All contributions are welcome!

## Disclaimer

This tool modifies the Roblox client at runtime, any modification to the client carries inherent risks.

-   This tool could break at any time due to Roblox updates.
-   You are using this tool at your own risk.
-   I am not responsible for any disciplinary action taken against your account (such as bans or warnings) or any other issues that may arise from its use.
