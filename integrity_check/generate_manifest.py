import hashlib
import os
import json
import hashlib
from datetime import datetime

def calculateSHA256(filePath: str) -> str:
    sha256_hash = hashlib.sha256()
    with open(filePath, 'rb') as file:
        for byteBlock in iter(lambda: file.read(4096), b""):
            sha256_hash.update(byteBlock)

    return sha256_hash.hexdigest()

def generateManifest(sourceDir: str):
    manifest = {
        "projectName": "Axiom",
        "timeGeneratedAt": datetime.utcnow().isoformat() + "Z",
        "files": []
    }

    allHashes = ""

    for root, dirs, files in os.walk(sourceDir):
        for f in sorted(files):
            if f.endswith(('.py', '.cpp', '.ini')):
                filePath = os.path.join(root, f)
                fileHash = calculateSHA256(filePath)

                cleanPath = os.path.relpath(filePath, sourceDir)

                manifest["files"].append({
                    "path": cleanPath,
                    "hash": fileHash
                })

                allHashes += fileHash

    manifest["masterHash"] = hashlib.sha256(allHashes.encode()).hexdigest()

    with open("../.manifest/manifest.json", "w") as manifestFile:
        json.dump(manifest, manifestFile, indent=4)

    print("Manifest created successfully!")
    print(f"Master hash: {manifest['masterHash']}")

if __name__ == "__main__":
    generateManifest("../src")
