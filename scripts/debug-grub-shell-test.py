#!/usr/bin/env python3
"""Boot GRUB, inject shell commands via QEMU monitor sendkey, verify debugcon."""
import json
import os
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
BOOT_SECS = int(os.environ.get("BOOT_SECS", "14"))


def mon_cmd(sock, cmd):
    sock.sendall((cmd + "\n").encode())
    sock.settimeout(2.0)
    try:
        return sock.recv(4096).decode("utf-8", "replace")
    except socket.timeout:
        return ""


def send_keys(sock, text):
    for ch in text:
        if ch == "\n":
            mon_cmd(sock, "sendkey ret")
        else:
            mon_cmd(sock, f"sendkey {ch}")
        time.sleep(0.05)


def main():
    subprocess.run(["make", "build", "grub-iso"], check=True, stdout=subprocess.DEVNULL)
    subprocess.run(["pkill", "-9", "qemu-system-x86_64"], stderr=subprocess.DEVNULL)
    time.sleep(0.2)
    os.makedirs(".cursor", exist_ok=True)
    try:
        os.unlink(MON_SOCK)
    except FileNotFoundError:
        pass
    open(TRACE, "w").close()
    open(DEBUG_LOG, "w").close()

    qemu = subprocess.Popen(
        [
            QEMU,
            "-drive", "file=bin/aitos.iso,format=raw,if=ide,index=0,media=disk",
            "-boot", "order=c,menu=off",
            "-m", "128", "-cpu", "qemu64", "-no-reboot",
            "-debugcon", f"file:{TRACE}",
            "-display", "none",
            "-serial", "none",
            "-monitor", f"unix:{MON_SOCK},server,nowait",
            "-nic", "none",
            "-device", "isa-debug-exit,iobase=0xf4,iosize=0x04",
        ],
        stdin=subprocess.DEVNULL,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    deadline = time.time() + BOOT_SECS + 20
    sock = None
    while time.time() < deadline:
        if os.path.exists(MON_SOCK):
            try:
                sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                sock.connect(MON_SOCK)
                break
            except OSError:
                pass
        if qemu.poll() is not None:
            break
        time.sleep(0.2)

    if sock:
        time.sleep(BOOT_SECS)
        send_keys(sock, "help\n")
        time.sleep(2)
        send_keys(sock, "version\n")
        time.sleep(2)
        send_keys(sock, "echo hello\n")
        time.sleep(2)
        sock.close()

    time.sleep(2)
    if qemu.poll() is None:
        qemu.kill()
        qemu.wait()

    text = open(TRACE, "rb").read().decode("utf-8", "replace")
    stages = [line for line in text.replace("\r", "").splitlines() if line.startswith("@")]
    with open(DEBUG_LOG, "w") as out:
        ts = int(time.time() * 1000)
        for s in stages:
            out.write(json.dumps({
                "sessionId": "536064",
                "hypothesisId": "stage",
                "location": "debugcon",
                "message": s,
                "data": {},
                "timestamp": ts,
                "runId": "sendkey-test",
            }) + "\n")

    checks = {
        "prompt": "aitos@localhost" in text,
        "help": "Available commands" in text,
        "version_cmd": "@version:done" in text,
        "echo": "hello" in text,
        "dispatch": "@repl:dispatch" in text,
        "help_enter": "@help:enter" in text,
        "help_done": "@help:done" in text,
    }
    print("stages:", stages)
    print("checks:", checks)
    for line in text.replace("\r", "").splitlines()[-40:]:
        print(line)
    return 0 if all(checks.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
