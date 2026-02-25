#!/usr/bin/env python3
"""
Capture a screenshot of the game window.
Usage: python3 screenshot.py [output_path]
Default output: Platform/MacOS/Build/Logs/screenshot.png
"""
import subprocess
import sys
import os
from datetime import datetime

def capture_window(output_path=None):
    """Capture the game window by name using screencapture."""
    if not output_path:
        logs_dir = os.path.join(os.path.dirname(__file__), "Logs")
        os.makedirs(logs_dir, exist_ok=True)
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        output_path = os.path.join(logs_dir, f"screenshot_{ts}.png")

    # Method 1: Try to capture specific window by title
    # Use AppleScript to find the window ID
    script = '''
    tell application "System Events"
        set wList to every window of every process whose name contains "generalszh"
        repeat with w in wList
            repeat with win in w
                return id of win
            end repeat
        end repeat
    end tell
    '''

    try:
        # Simpler approach: capture by window title match using screencapture -l
        # First find the window ID via CGWindowListCopyWindowInfo
        import json
        result = subprocess.run(
            ["python3", "-c", """
import Quartz
import json
windows = Quartz.CGWindowListCopyWindowInfo(
    Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID
)
for w in windows:
    name = w.get('kCGWindowOwnerName', '')
    title = w.get('kCGWindowName', '')
    if 'generalszh' in name.lower() or 'generals' in title.lower() or 'command' in title.lower():
        print(json.dumps({
            'id': w['kCGWindowNumber'],
            'owner': name,
            'title': title,
            'bounds': dict(w.get('kCGWindowBounds', {}))
        }))
        break
"""],
            capture_output=True, text=True
        )

        if result.stdout.strip():
            info = json.loads(result.stdout.strip())
            wid = info['id']
            print(f"Found window: {info['owner']} - '{info['title']}' (id={wid})")
            # Capture by window ID
            subprocess.run(["screencapture", "-l", str(wid), "-x", output_path], check=True)
            print(f"Screenshot saved: {output_path}")
            return output_path
    except Exception as e:
        print(f"Window capture failed: {e}")

    # Fallback: full screen capture
    print("Falling back to full screen capture...")
    subprocess.run(["screencapture", "-x", output_path], check=True)
    print(f"Screenshot saved: {output_path}")
    return output_path


if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else None
    capture_window(path)
