#!/usr/bin/env python3
"""Generate OpenKAI's original, texture-free multirotor glTF 2.0 model.

Metres; model axes are +X forward, +Y up, +Z right. The cyan nose
identifies the forward end. No third-party model or Python packages are used.
"""

import argparse
import json
import math
from pathlib import Path
import struct


def box(sx, sy, sz):
    vertices = [(-sx/2, -sy/2, -sz/2), (sx/2, -sy/2, -sz/2),
                (sx/2, sy/2, -sz/2), (-sx/2, sy/2, -sz/2),
                (-sx/2, -sy/2, sz/2), (sx/2, -sy/2, sz/2),
                (sx/2, sy/2, sz/2), (-sx/2, sy/2, sz/2)]
    faces = [(0, 3, 2, 1), (4, 5, 6, 7), (0, 4, 7, 3),
             (1, 2, 6, 5), (0, 1, 5, 4), (3, 7, 6, 2)]
    return [vertices[i] for face in faces for i in
            (face[0], face[1], face[2], face[0], face[2], face[3])]


def cylinder(radius, height, segments=24):
    triangles = []
    for i in range(segments):
        a, b = i * math.tau / segments, (i + 1) * math.tau / segments
        p = (radius * math.cos(a), -height/2, radius * math.sin(a))
        q = (radius * math.cos(b), -height/2, radius * math.sin(b))
        r, s = (p[0], height/2, p[2]), (q[0], height/2, q[2])
        triangles.extend([p, r, q, q, r, s, (0, -height/2, 0), p, q,
                          (0, height/2, 0), s, r])
    return triangles


def create_model(destination):
    gltf = {"asset": {"version": "2.0", "generator": "OpenKAI create_drone.py",
                       "copyright": "OpenKAI contributors; same license as OpenKAI"},
            "scene": 0, "scenes": [{"nodes": []}], "nodes": [], "meshes": [],
            "accessors": [], "bufferViews": [], "buffers": [], "materials": [],
            "extras": {"units": "metres", "axes": "+X forward, +Y up, +Z right",
                       "nose": "cyan", "source": "original procedural geometry"}}
    colors = [(0.065, 0.09, 0.13, 1), (0.24, 0.31, 0.40, 1),
              (0.10, 0.80, 0.91, 1), (1.0, 0.34, 0.08, 1),
              (0.025, 0.035, 0.045, 1), (0.76, 0.80, 0.84, 1)]
    for i, color in enumerate(colors):
        material = {"name": ["carbon", "aluminum", "nose-cyan", "rear-orange",
                             "propellers", "landing-gear"][i],
                    "pbrMetallicRoughness": {"baseColorFactor": color,
                        "metallicFactor": 0.25, "roughnessFactor": 0.65}}
        if i in (2, 3):
            material["emissiveFactor"] = [v * 0.25 for v in color[:3]]
        gltf["materials"].append(material)
    binary = bytearray()

    def accessor(vectors):
        index = len(gltf["accessors"])
        flat = [v for vec in vectors for v in vec]
        data = struct.pack("<" + "f" * len(flat), *flat)
        gltf["bufferViews"].append({"buffer": 0, "byteOffset": len(binary),
                                    "byteLength": len(data), "target": 34962})
        binary.extend(data)
        gltf["accessors"].append({"bufferView": index, "componentType": 5126,
            "count": len(vectors), "type": "VEC3",
            "min": [min(v[i] for v in vectors) for i in range(3)],
            "max": [max(v[i] for v in vectors) for i in range(3)]})
        return index

    def part(name, triangles, material, position=(0, 0, 0), yaw=0):
        normals = []
        for i in range(0, len(triangles), 3):
            a, b, c = triangles[i:i+3]
            u, v = [b[j] - a[j] for j in range(3)], [c[j] - a[j] for j in range(3)]
            n = (u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0])
            length = math.sqrt(sum(x*x for x in n))
            normals.extend([tuple(x/length for x in n)] * 3)
        mesh = len(gltf["meshes"])
        gltf["meshes"].append({"name": name, "primitives": [{"attributes": {
            "POSITION": accessor(triangles), "NORMAL": accessor(normals)},
            "material": material}]})
        gltf["scenes"][0]["nodes"].append(len(gltf["nodes"]))
        gltf["nodes"].append({"name": name, "mesh": mesh,
            "translation": position, "rotation": [0, math.sin(yaw/2), 0, math.cos(yaw/2)]})

    part("fuselage", box(0.62, 0.18, 0.36), 0)
    part("battery", box(0.40, 0.10, 0.28), 1, (0, 0.13, 0))
    part("nose", box(0.08, 0.12, 0.30), 2, (0.34, 0, 0))
    part("tail", box(0.04, 0.08, 0.28), 3, (-0.33, 0, 0))
    part("gps", cylinder(0.08, 0.025), 5, (-0.12, 0.20, 0))
    part("camera", box(0.10, 0.10, 0.13), 1, (0.30, -0.15, 0))
    for x in (-0.49, 0.49):
        for z in (-0.49, 0.49):
            label = ("front" if x > 0 else "rear") + ("_right" if z > 0 else "_left")
            part("arm_" + label, box(math.hypot(x, z), 0.05, 0.065), 1,
                 (x/2, 0, z/2), -math.atan2(z, x))
            part("motor_" + label, cylinder(0.065, 0.11), 0, (x, 0.045, z))
            part("rotor_" + label, box(0.52, 0.009, 0.035), 4,
                 (x, 0.108, z), math.pi/4 if x*z > 0 else -math.pi/4)
            part("hub_" + label, cylinder(0.025, 0.035, 16), 2 if x > 0 else 3,
                 (x, 0.12, z))
    for z in (-0.21, 0.21):
        part("landing_skid", box(0.72, 0.035, 0.035), 5, (0, -0.28, z))
        for x in (-0.20, 0.20):
            part("landing_leg", box(0.027, 0.20, 0.027), 5, (x, -0.18, z))
    gltf["buffers"].append({"byteLength": len(binary)})
    document = json.dumps(gltf, separators=(",", ":")).encode()
    document += b" " * (-len(document) % 4)
    binary.extend(b"\0" * (-len(binary) % 4))
    output = struct.pack("<III", 0x46546c67, 2, 12 + 8 + len(document) + 8 + len(binary))
    output += struct.pack("<II", len(document), 0x4e4f534a) + document
    output += struct.pack("<II", len(binary), 0x004e4942) + binary
    destination = Path(destination)
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(output)
    return destination


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", nargs="?", type=Path, default=Path("multirotor.glb"))
    args = parser.parse_args()
    print(create_model(args.output))
