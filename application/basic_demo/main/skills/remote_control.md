# Remote Control 远程控制

Control and monitor the ESP-Claw device remotely through the built-in web terminal, Lua script runner, or Telegram bot CLI.

## When to use
When the user wants to:
- Execute a CLI command on the device remotely
- Run a Lua script for automation or testing
- Check device status, memory, files, or logs without physical access
- Interact with the device via Telegram chat

## Capabilities
- **Web Terminal**: Real-time CLI via browser at the device's IP address
- **Lua Runner**: Execute `.lua` scripts from `/scripts` or write inline code
- **Telegram CLI**: Send `/cmd <command>` to the bot for direct execution

## How to use

### Web Terminal
1. Open the device's web UI (usually `http://<device-ip>/`)
2. Switch to the **Terminal** tab
3. Type any registered CLI command (e.g., `help`, `free`, `version`)
4. Press Enter or click Run — output appears in the terminal panel

### Lua Runner
1. Open the **Lua** tab in the web UI
2. Select an existing script from the file list, or write code in the editor
3. Click **Run** — output and any errors are displayed below

### Telegram CLI
1. Send a message to the bot starting with `/cmd ` followed by the command
   - Example: `/cmd free`
   - Example: `/cmd ls /scripts`
2. The bot replies with the command output directly in chat
3. Note: long outputs may be truncated to fit Telegram message limits

## Example
User: "Run the status command on my device"
-> Open web Terminal tab (or tell user to use Telegram `/cmd status`)
-> Execute `status` or `free` or `version`
-> Return output to user
