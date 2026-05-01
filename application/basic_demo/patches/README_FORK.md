# How to Fork and Push

## 1. Create the fork on GitHub

Go to https://github.com/espressif/esp-claw and click **Fork** (top-right).  
Choose your personal account as the destination.

## 2. Add your fork as a remote

In the ESP-IDF terminal (inside `application/basic_demo`):

```bash
git remote add fork https://github.com/TUO_USERNAME/esp-claw.git
```

## 3. Push the branch

```bash
git push -u fork feat/esp32s3-wt32-sc01-plus
```

## 4. Board Manager Patch

The `managed_components/` folder is `.gitignore`d, so the I80 LCD patch **is not included in the commit**.  
You have two options:

### Option A – Apply locally after clone (recommended)
Anyone who clones your fork must manually apply the board-manager patch described in `patches/ESP_BOARD_MANAGER_I80_PATCH.md`.

### Option B – Commit the patched board manager
If you want the fork to work out-of-the-box, temporarily remove `managed_components/` from `.gitignore`, commit the patched files, push, and then restore `.gitignore`:

```bash
git rm --cached .gitignore
# edit .gitignore: comment out or remove the "managed_components/" line
git add .gitignore
git add -f managed_components/espressif__esp_board_manager/devices/dev_display_lcd/
git add -f managed_components/espressif__esp_board_manager/devices/CMakeLists.txt
git add -f managed_components/espressif__esp_board_manager/gen_codes/Kconfig.in
git commit -m "patch(board_manager): add i80 lcd support"
git push fork feat/esp32s3-wt32-sc01-plus
```

> **Warning:** Option B makes the repository much heavier because it version-controls all managed components. Use only if you need a fully self-contained fork.

## 5. Pull Request (optional)

If you want to upstream the board definition (without the board-manager patch), open a PR from `TUO_USERNAME/esp-claw:feat/esp32s3-wt32-sc01-plus` to `espressif/esp-claw:master`.
