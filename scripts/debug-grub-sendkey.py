#!/usr/bin/env python3
"""Send QEMU monitor commands via -monitor unix socket after GRUB shell boots."""
import json
import os
import re
import socket
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)

TRACE = "bin/boot-debugcon.log"
DEBUG_LOG = ".cursor/debug-536064.log"
MON_SOCK = "/tmp/aitos-qemu-mon.sock"
QEMU = os.environ.get("QEMU", "qemu-system-x86_64")

subprocess.run(["make", "build", "grub-iso"], check=True, stdout=subprocess.DEVNULL)
subprocess.run(["pkill", "-9", "qemu-system-x86_64"], stderr=subprocess.DEVNULL)
time.sleep(0.2)
os.makedirs(".cursor", exist_ok=True)
open(TRACE, "w").close()
open(DEBUG_LOG, "w").close()
if os.path.exists(MON_SOCK):
    os.remove(MON_SOCK)

qemu = subprocess.Popen(
    [
        QEMU,
        "-drive", "file=bin/aitos.iso,format=raw,if=ide,index=0,media=disk",
        "-boot", "order=c,menu=off",
        "-m", "128", "-cpu", "qemu64", "-no-reboot",
        "-debugcon", f"file:{TRACE}",
        "-display", "none", "-nic", "none",
        "-monitor", f"unix:{MON_SOCK},server,nowait",
        "-device", "isa-debug-exit,iobase=0xf4,iosize=0x04",
    ],
    stdin=subprocess.DEVNULL,
    stdout=subprocess.DEVNULL,
    stderr=subprocess.DEVNULL,
)

for _ in range(80):
    if os.path.exists(TRACE) and "aitos@localhost" in open(TRACE, "rb").read().decode("utf-8", "replace"):
        break
    time.sleep(0.25)

time.sleep(0.5)

def mon_cmd(cmd):
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.connect(MON_SOCK)
    s.recv(4096)
    s.sendall((cmd + "\n").encode())
    time.sleep(0.15)
    out = s.recv(8192).decode("utf-8", "replace")
    s.close()
    return out

for key in ["h", "e", "l", "p", "ret"]:
    mon_cmd(f"sendkey {key}")
    time.sleep(0.1)
time.sleep(4)
for seq in ["v", "e", "r", "s", "i", "o", "n", "ret"]:
    mon_cmd(f"sendkey {seq}")
    time.sleep(0.08)
time.sleep(3)

qemu.kill()
qemu.wait()
try:
    os.remove(MON_SOCK)
except OSError:
    pass

text = open(TRACE, "rb").read().decode("utf-8", "replace")
pat = re.compile(r"DBG\|536064\|([^|]+)\|([^|]+)\|([^|]+)\|([0-9a-f]+)")
markers = pat.findall(text)
with open(DEBUG_LOG, "w") as out:
    ts = int(time.time() * 1000)
    for h, loc, msg, val in markers:
        out.write(json.dumps({
            "sessionId": "536064",
            "hypothesisId": h,
            "location": loc,
            "message": msg,
            "data": {"value": int(val, 16)},
            "timestamp": ts,
            "runId": "sendkey-test",
        }) + "\n")

print(f"markers: {len(markers)}")
print(f"Available: {'Available commands' in text}")
print(f"version: {'0.1.0-mvp' in text}")
for line in text.replace("\r", "").splitlines()[-30:]:
    print(line)
