#!/usr/bin/env python3
"""Convert a colored STEP assembly to a self-contained, metre-scale glTF 2.0 GLB.

Install the optional converter in an isolated environment (not needed to view):
    python3 -m venv /tmp/openkai-step-venv
    /tmp/openkai-step-venv/bin/pip install cadquery-ocp==8.0.1.0.0 numpy

The default axes/pivot match the supplied Hydrone STEP: -Y forward, +Z up,
origin at the motor array. All parts are retained unless the explicit
--exclude-hydrone-payload option removes the verified cardboard box only.
The GLB uses the viewer's +X forward, +Y up, +Z right convention.
"""

import argparse
import hashlib
import importlib.metadata
import json
import math
from pathlib import Path
import struct
import tempfile


HYDRONE_SOURCE_SHA256 = 'a8fb2ce80a7a66edd7d5924fe39ea503ace71729dd6c430340b59c1e70eda072'


def sha256(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def read_glb(path):
    data = path.read_bytes()
    magic, version, length = struct.unpack_from('<4sII', data)
    if magic != b'glTF' or version != 2 or length != len(data):
        raise ValueError('Invalid glTF 2.0 binary container')
    chunks = []
    offset = 12
    while offset < length:
        size, kind = struct.unpack_from('<II', data, offset)
        offset += 8
        chunks.append((kind, data[offset:offset + size]))
        offset += size
    if offset != length or len(chunks) != 2 or [c[0] for c in chunks] != [0x4e4f534a, 0x004e4942]:
        raise ValueError('Expected one JSON and one embedded BIN chunk')
    return json.loads(chunks[0][1]), chunks[1][1]


def write_glb(path, document, binary):
    encoded = json.dumps(document, separators=(',', ':'), ensure_ascii=False).encode('utf-8')
    encoded += b' ' * (-len(encoded) % 4)
    binary += b'\0' * (-len(binary) % 4)
    data = (struct.pack('<4sII', b'glTF', 2, 28 + len(encoded) + len(binary))
            + struct.pack('<II', len(encoded), 0x4e4f534a) + encoded
            + struct.pack('<II', len(binary), 0x004e4942) + binary)
    path.write_bytes(data)


def accessor_bytes(document, binary, index):
    """Read the tightly packed value bytes independently of buffer layout."""
    item = document['accessors'][index]
    view = document['bufferViews'][item['bufferView']]
    components = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3, 'VEC4': 4}[item['type']]
    width = components * {5121: 1, 5123: 2, 5125: 4, 5126: 4}[item['componentType']]
    stride = view.get('byteStride', width)
    offset = view.get('byteOffset', 0) + item.get('byteOffset', 0)
    if stride == width:
        return binary[offset:offset + item['count'] * width]
    return b''.join(binary[offset + i * stride:offset + i * stride + width]
                    for i in range(item['count']))


def geometry_fingerprint(document, binary, omit=None):
    """Hash retained primitive names, materials and exact vertex/index bytes."""
    digest = hashlib.sha256()
    for mi, mesh in enumerate(document['meshes']):
        for pi, primitive in enumerate(mesh['primitives']):
            if (mi, pi) == omit:
                continue
            material = document['materials'][primitive['material']]
            digest.update(json.dumps([mesh.get('name'), primitive.get('mode', 4), material],
                                     sort_keys=True).encode('utf-8'))
            attributes = {**primitive['attributes'], 'indices': primitive['indices']}
            for name, index in sorted(attributes.items()):
                item = document['accessors'][index]
                digest.update(json.dumps([name, item['type'], item['componentType'], item['count']]).encode('ascii'))
                digest.update(accessor_bytes(document, binary, index))
    return digest.hexdigest()


def exclude_hydrone_payload(document, binary):
    """Remove the fingerprint-guarded source's isolated box, never aircraft parts.

    STEP #642 (Körper33) exports as one distinct brown COMPOUND primitive.
    Match its complete box geometry rather than relying on unstable mesh indices.
    Aircraft and gripping clamps use different primitives, verified byte-for-byte.
    """
    import itertools
    import numpy as np

    lower, upper = np.array([-.02, -.1, -.3]), np.array([.02, .1, -.1])
    expected = np.array(list(itertools.product(*zip(lower, upper))))
    matches = []
    for mi, mesh in enumerate(document['meshes']):
        if mesh.get('name') != 'COMPOUND':
            continue
        for pi, primitive in enumerate(mesh['primitives']):
            a = document['accessors'][primitive['attributes']['POSITION']]
            indices = document['accessors'][primitive['indices']]
            if a['type'] != 'VEC3' or a['componentType'] != 5126 or a['count'] != 24 or indices['count'] != 36:
                continue
            points = np.frombuffer(accessor_bytes(document, binary, primitive['attributes']['POSITION']), dtype='<f4').reshape(-1, 3)
            corners = np.unique(points, axis=0)
            if corners.shape != (8, 3) or not np.allclose(corners, expected, atol=1e-7, rtol=0):
                continue
            material = document['materials'][primitive['material']]
            color = material['pbrMetallicRoughness']['baseColorFactor']
            if not np.allclose(color, [.48514995, .33245152, .14702727, 1], atol=1e-6, rtol=0):
                continue
            # Confirm a closed cuboid: every triangle is on one of its six faces,
            # and every welded edge is shared by exactly two triangles.
            dtype = {5121: '<u1', 5123: '<u2', 5125: '<u4'}[indices['componentType']]
            triangles = np.frombuffer(accessor_bytes(document, binary, primitive['indices']), dtype=dtype).reshape(-1, 3)
            corner_ids = [int(np.argmin(np.linalg.norm(corners - p, axis=1))) for p in points]
            edges = {}
            for triangle in triangles:
                face = points[triangle]
                if not np.any(np.ptp(face, axis=0) < 1e-7):
                    raise ValueError('Payload candidate contains a non-box face')
                welded = [corner_ids[int(i)] for i in triangle]
                if len(set(welded)) != 3:
                    raise ValueError('Payload candidate contains a degenerate triangle')
                for i in range(3):
                    edge = tuple(sorted((welded[i], welded[(i + 1) % 3])))
                    edges[edge] = edges.get(edge, 0) + 1
            if len(edges) != 18 or set(edges.values()) != {2}:
                raise ValueError('Payload candidate is not a closed 12-triangle box')
            matches.append((mi, pi))
    if len(matches) != 1:
        raise ValueError(f'Expected exactly one verified Hydrone payload box, found {len(matches)}')
    removed = matches[0]
    retained_before = geometry_fingerprint(document, binary, omit=removed)
    del document['meshes'][removed[0]]['primitives'][removed[1]]

    # Remove orphaned accessors/materials and trim the three packed buffer tails.
    # This also physically removes payload vertices and indices from the GLB.
    used_accessors, used_materials = set(), set()
    for mesh in document['meshes']:
        for primitive in mesh['primitives']:
            used_accessors.update(primitive['attributes'].values())
            used_accessors.add(primitive['indices'])
            used_materials.add(primitive['material'])
    accessor_map = {old: new for new, old in enumerate(sorted(used_accessors))}
    material_map = {old: new for new, old in enumerate(sorted(used_materials))}
    document['accessors'] = [document['accessors'][old] for old in sorted(used_accessors)]
    document['materials'] = [document['materials'][old] for old in sorted(used_materials)]
    view_ends = {}
    for item in document['accessors']:
        view_id = item['bufferView']
        view = document['bufferViews'][view_id]
        width = ({'SCALAR': 1, 'VEC2': 2, 'VEC3': 3, 'VEC4': 4}[item['type']]
                 * {5121: 1, 5123: 2, 5125: 4, 5126: 4}[item['componentType']])
        end = item.get('byteOffset', 0) + (item['count'] - 1) * view.get('byteStride', width) + width
        view_ends[view_id] = max(end, view_ends.get(view_id, 0))
    packed = bytearray()
    for view_id, view in enumerate(document['bufferViews']):
        if view_id not in view_ends:
            raise ValueError('Unexpected wholly unused buffer view during payload exclusion')
        start = view.get('byteOffset', 0)
        end = view_ends[view_id]
        packed.extend(b'\0' * (-len(packed) % 4))
        view['byteOffset'] = len(packed)
        view['byteLength'] = end
        packed.extend(binary[start:start + end])
    for mesh in document['meshes']:
        for primitive in mesh['primitives']:
            primitive['attributes'] = {name: accessor_map[index] for name, index in primitive['attributes'].items()}
            primitive['indices'] = accessor_map[primitive['indices']]
            primitive['material'] = material_map[primitive['material']]
    document['buffers'][0]['byteLength'] = len(packed)
    retained_after = geometry_fingerprint(document, packed)
    if retained_before != retained_after:
        raise ValueError('Payload exclusion unexpectedly altered retained aircraft geometry or materials')
    return bytes(packed), {
        'sourceStepEntity': 642, 'sourceName': 'Körper33', 'description': 'Cardboard payload box only',
        'nativeBoundsMillimeters': {'minimum': (lower * 1000).tolist(), 'maximum': (upper * 1000).tolist()},
        'trianglesRemoved': 12, 'verticesRemoved': 24, 'uniqueCorners': 8,
        'binaryBytesRemoved': len(binary) - len(packed),
        'retainedGeometryAndMaterialsSha256': retained_after,
        'verification': 'All retained primitive vertex/index bytes, names and materials match the complete assembly.',
        'preservedStepEntities': [640, 641],
    }


def model_stats(document, binary):
    """Validate geometry and measure actual transformed vertices, in metres."""
    import numpy as np

    if len(document['buffers']) != 1 or document['buffers'][0].get('uri'):
        raise ValueError('The viewer model must have one embedded buffer')
    if document['buffers'][0]['byteLength'] > len(binary):
        raise ValueError('Truncated binary geometry')
    if document.get('extensionsRequired'):
        raise ValueError('Unexpected required glTF extension')
    types = {5121: '<u1', 5123: '<u2', 5125: '<u4', 5126: '<f4'}
    sizes = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3, 'VEC4': 4}

    def accessor(index):
        item = document['accessors'][index]
        view = document['bufferViews'][item['bufferView']]
        dtype = np.dtype(types[item['componentType']])
        columns = sizes[item['type']]
        offset = view.get('byteOffset', 0) + item.get('byteOffset', 0)
        stride = view.get('byteStride', columns * dtype.itemsize)
        result = np.ndarray((item['count'], columns), dtype=dtype,
                            buffer=binary, offset=offset, strides=(stride, dtype.itemsize))
        if not np.isfinite(result).all():
            raise ValueError('Non-finite geometry')
        return result

    positions = {}
    unique_triangles = 0
    primitives = 0
    for index, mesh in enumerate(document.get('meshes', [])):
        positions[index] = []
        for primitive in mesh['primitives']:
            if primitive.get('mode', 4) != 4:
                raise ValueError('Only triangulated surfaces are expected')
            points = accessor(primitive['attributes']['POSITION'])
            positions[index].append(points)
            indices = accessor(primitive['indices']) if 'indices' in primitive else None
            count = len(indices) if indices is not None else len(points)
            if count % 3 or (indices is not None and indices.max() >= len(points)):
                raise ValueError('Invalid triangle indices')
            unique_triangles += count // 3
            primitives += 1

    def transform(node):
        if 'matrix' in node:
            return np.array(node['matrix'], dtype=float).reshape(4, 4).T
        result = np.eye(4)
        if 'rotation' in node:
            x, y, z, w = node['rotation']
            result[:3, :3] = [
                [1 - 2 * (y*y + z*z), 2 * (x*y - w*z), 2 * (x*z + w*y)],
                [2 * (x*y + w*z), 1 - 2 * (x*x + z*z), 2 * (y*z - w*x)],
                [2 * (x*z - w*y), 2 * (y*z + w*x), 1 - 2 * (x*x + y*y)],
            ]
        result[:3, :3] = result[:3, :3] @ np.diag(node.get('scale', [1, 1, 1]))
        result[:3, 3] = node.get('translation', [0, 0, 0])
        return result

    lower = np.full(3, np.inf)
    upper = np.full(3, -np.inf)
    instances = 0
    rendered_triangles = 0

    def visit(index, parent, ancestors):
        nonlocal lower, upper, instances, rendered_triangles
        if index in ancestors:
            raise ValueError('Cyclic scene graph')
        node = document['nodes'][index]
        world = parent @ transform(node)
        if 'mesh' in node:
            instances += 1
            for points in positions[node['mesh']]:
                transformed = points @ world[:3, :3].T + world[:3, 3]
                lower = np.minimum(lower, transformed.min(axis=0))
                upper = np.maximum(upper, transformed.max(axis=0))
            for primitive in document['meshes'][node['mesh']]['primitives']:
                a = primitive.get('indices', primitive['attributes']['POSITION'])
                rendered_triangles += document['accessors'][a]['count'] // 3
        for child in node.get('children', []):
            visit(child, world, ancestors | {index})

    for root in document['scenes'][document.get('scene', 0)]['nodes']:
        visit(root, np.eye(4), set())
    if not np.isfinite(lower).all():
        raise ValueError('No model geometry was exported')
    return {
        'boundsMeters': {'minimum': lower.tolist(), 'maximum': upper.tolist()},
        'dimensionsMeters': (upper - lower).tolist(),
        'meshes': len(document.get('meshes', [])), 'meshInstances': instances,
        'nodes': len(document.get('nodes', [])), 'materials': len(document.get('materials', [])),
        'primitives': primitives, 'uniqueTriangles': unique_triangles,
        'renderedTriangles': rendered_triangles,
    }


def convert(args):
    import numpy as np
    from OCP.BRepBndLib import BRepBndLib
    from OCP.BRepMesh import BRepMesh_IncrementalMesh
    from OCP.Bnd import Bnd_Box
    from OCP.IFSelect import IFSelect_RetDone
    from OCP.Message import Message_ProgressRange
    from OCP.RWGltf import RWGltf_CafWriter
    from OCP.RWMesh import RWMesh_CoordinateSystem_Yup
    from OCP.STEPCAFControl import STEPCAFControl_Reader
    from OCP.TCollection import TCollection_AsciiString, TCollection_ExtendedString
    from OCP.TDataStd import TDataStd_Name
    from OCP.TDF import TDF_Label
    from OCP.TDocStd import TDocStd_Document
    from OCP.XCAFDoc import XCAFDoc_DocumentTool
    from OCP.collections import (IndexedDataMap_TCollection_AsciiString_TCollection_AsciiString,
                                 Sequence_TCollection_AsciiString, Sequence_TDF_Label)

    def axis(value):
        vector = np.zeros(3)
        vector['xyz'.index(value[-1])] = -1 if value.startswith('-') else 1
        return vector

    forward, up = axis(args.forward_axis), axis(args.up_axis)
    if np.dot(forward, up):
        raise ValueError('Forward and up axes must be perpendicular')
    rotation = np.array([forward, up, np.cross(forward, up)])
    pivot = np.array(args.pivot_mm, dtype=float)
    source = args.input.resolve()
    destination = args.output.resolve()
    if source == destination or destination.suffix.lower() != '.glb':
        raise ValueError('Output must be a separate .glb file')
    source_sha = sha256(source)
    if args.exclude_hydrone_payload and source_sha != HYDRONE_SOURCE_SHA256:
        raise ValueError('--exclude-hydrone-payload is restricted to the verified original Hydrone STEP SHA256')
    destination.parent.mkdir(parents=True, exist_ok=True)
    print('Reading STEP assembly and its colors...', flush=True)
    reader = STEPCAFControl_Reader()
    reader.SetColorMode(True)
    reader.SetNameMode(True)
    reader.SetMatMode(True)
    if reader.ReadFile(str(source)) != IFSelect_RetDone:
        raise ValueError('STEP reader failed')
    units, angles, solids = [Sequence_TCollection_AsciiString() for _ in range(3)]
    reader.Reader().FileUnits(units, angles, solids)
    # OCCT's system length unit is expressed in mm: 1.0 explicitly imports mm.
    # STEP source units are converted by the reader before tessellation.
    reader.ChangeReader().SetSystemLengthUnit(1.0)
    document = TDocStd_Document(TCollection_ExtendedString('BinXCAF'))
    if not reader.Transfer(document):
        raise ValueError('STEP assembly transfer failed')
    shape_tool = XCAFDoc_DocumentTool.ShapeTool_s(document.Main())
    roots = Sequence_TDF_Label()
    shape_tool.GetFreeShapes(roots)
    if not roots.Length():
        raise ValueError('No assembly roots in STEP')
    native_bounds = Bnd_Box()
    part_names = []
    print('Tessellating assembly...', flush=True)
    for index in range(1, roots.Length() + 1):
        root = roots.Value(index)
        shape = shape_tool.GetShape_s(root)
        BRepBndLib.AddOptimal_s(shape, native_bounds, False)
        mesh = BRepMesh_IncrementalMesh(shape, args.linear_deflection_mm, False,
                                       math.radians(args.angular_deflection_degrees), True)
        if not mesh.IsDone():
            raise ValueError('Assembly tessellation failed')
        components = Sequence_TDF_Label()
        shape_tool.GetComponents_s(root, components, True)
        for component in range(1, components.Length() + 1):
            reference = TDF_Label()
            shape_tool.GetReferredShape_s(components.Value(component), reference)
            name = TDataStd_Name()
            if reference.FindAttribute(TDataStd_Name.GetID_s(), name):
                part_names.append(name.Get().ToExtString())

    print('Writing binary glTF...', flush=True)
    with tempfile.TemporaryDirectory(prefix='step-glb-', dir=destination.parent) as temporary:
        raw = Path(temporary) / 'raw.glb'
        writer = RWGltf_CafWriter(TCollection_AsciiString(str(raw)), True)
        writer.SetMergeFaces(True)
        writer.SetParallel(True)
        converter = writer.ChangeCoordinateSystemConverter()
        converter.SetInputLengthUnit(0.001)
        converter.SetOutputLengthUnit(1.0)
        # Preserve imported CAD axes here; a named scene root applies the explicit
        # axis/pivot mapping below instead of relying on an implicit up-axis swap.
        converter.SetInputCoordinateSystem(RWMesh_CoordinateSystem_Yup)
        converter.SetOutputCoordinateSystem(RWMesh_CoordinateSystem_Yup)
        if not writer.Perform(document, IndexedDataMap_TCollection_AsciiString_TCollection_AsciiString(),
                              Message_ProgressRange()):
            raise ValueError('glTF writer failed')
        gltf, binary = read_glb(raw)

    exclusions = []
    if args.exclude_hydrone_payload:
        binary, excluded = exclude_hydrone_payload(gltf, binary)
        exclusions.append(excluded)

    matrix = np.eye(4)
    matrix[:3, :3] = rotation
    matrix[:3, 3] = -rotation @ (pivot * 0.001)
    for scene in gltf['scenes']:
        root_index = len(gltf['nodes'])
        gltf['nodes'].append({'name': 'Flight frame: X forward, Y up, Z right',
                              'matrix': matrix.T.flatten().tolist(), 'children': scene['nodes']})
        scene['nodes'] = [root_index]
    # STEP AP214 often supplies only surface colors. glTF's implicit metallic=1
    # would turn white plastic into unlit metal without an environment map.
    # Retain any authored PBR values; give color-only CAD surfaces matte defaults.
    for material in gltf.get('materials', []):
        surface = material.setdefault('pbrMetallicRoughness', {})
        surface.setdefault('metallicFactor', 0.0)
        surface.setdefault('roughnessFactor', 0.65)
    provenance = {
        'source': str(source), 'sourceSha256': source_sha, 'sourceBytes': source.stat().st_size,
        'sourceUnitNames': [units.Value(i).ToCString() for i in range(1, units.Length() + 1)],
        'sourceRights': 'User-provided CAD; original ownership and licensing retained.',
        'format': 'glTF 2.0 binary (GLB)', 'cadqueryOcpVersion': importlib.metadata.version('cadquery-ocp'),
        'linearDeflectionMillimeters': args.linear_deflection_mm,
        'angularDeflectionDegrees': args.angular_deflection_degrees,
        'importUnit': 'millimeter', 'outputUnit': 'meter', 'scaleToMeters': 0.001,
        'sourceForwardAxis': args.forward_axis, 'sourceUpAxis': args.up_axis,
        'sourcePivotMillimeters': pivot.tolist(),
        'outputAxes': {'forward': '+X', 'up': '+Y', 'right': '+Z'},
        'nativeBoundsMillimeters': {'minimum': list(native_bounds.CornerMin().Coord()),
                                    'maximum': list(native_bounds.CornerMax().Coord())},
        'nativeBoundsScope': 'Complete original STEP assembly before optional exclusions, in CAD axes',
        'boundsMetersScope': 'Exported scene after optional exclusions, in viewer axes',
        'rootTransformColumnMajor': matrix.T.flatten().tolist(),
        'colorOnlyMaterialDefaults': {'metallicFactor': 0.0, 'roughnessFactor': 0.65},
        'assemblyParts': part_names,
        'preserved': ['assembly hierarchy', 'aircraft and gripping clamps' if exclusions else 'all parts including payload and clamps', 'CAD colors', 'physical scale'],
        'excludedParts': exclusions,
    }
    gltf.setdefault('asset', {}).setdefault('extras', {})['conversion'] = {
        key: provenance[key] for key in ('sourceSha256', 'outputUnit', 'sourceForwardAxis',
                                         'sourceUpAxis', 'sourcePivotMillimeters', 'outputAxes', 'excludedParts')}
    provenance.update(model_stats(gltf, binary))
    write_glb(destination, gltf, binary)
    # Read the written container back, independently of the in-memory document.
    model_stats(*read_glb(destination))
    provenance.update({'outputFile': destination.name, 'outputBytes': destination.stat().st_size,
                        'outputSha256': sha256(destination)})
    report = args.report or destination.with_suffix('.conversion.json')
    report.write_text(json.dumps(provenance, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    print(json.dumps({key: provenance[key] for key in ('outputFile', 'outputBytes', 'dimensionsMeters',
                                                       'meshes', 'materials', 'renderedTriangles')}, indent=2))
    print(f'Conversion report: {report}')


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--input', type=Path, required=True, help='Source STEP/STP assembly')
    parser.add_argument('--output', type=Path, required=True, help='Destination self-contained .glb file')
    parser.add_argument('--report', type=Path, help='Provenance JSON (default: OUTPUT.conversion.json)')
    parser.add_argument('--linear-deflection-mm', type=float, default=0.15, help='Absolute tessellation tolerance in mm (default 0.15)')
    parser.add_argument('--angular-deflection-degrees', type=float, default=20, help='Angular tessellation tolerance (default 20 degrees)')
    parser.add_argument('--forward-axis', choices=['x', '-x', 'y', '-y', 'z', '-z'], default='-y')
    parser.add_argument('--up-axis', choices=['x', '-x', 'y', '-y', 'z', '-z'], default='z')
    parser.add_argument('--pivot-mm', type=float, nargs=3, default=[0, 0, 0], metavar=('X', 'Y', 'Z'))
    parser.add_argument('--exclude-hydrone-payload', action='store_true',
                        help='Remove only the verified original Hydrone cardboard box (STEP #642); retain clamps')
    args = parser.parse_args()
    if not 0 < args.linear_deflection_mm < 100 or not 0 < args.angular_deflection_degrees < 90:
        parser.error('Tessellation tolerances must be positive (linear < 100 mm, angular < 90 degrees)')
    if not all(math.isfinite(value) for value in args.pivot_mm):
        parser.error('Pivot coordinates must be finite')
    convert(args)


if __name__ == '__main__':
    main()
