#!/usr/bin/env python3
"""Run with python3 html/console/editor/tests/schema.test.py."""
import importlib.util
from pathlib import Path
import re
import unittest


generator_path = Path(__file__).resolve().parents[1] / "tools" / "generate-schema.py"
spec = importlib.util.spec_from_file_location("openkai_schema", generator_path)
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


class InitializerDefaultTest(unittest.TestCase):
    def test_local_copies_resolve_source_defaults(self):
        initializers = {
            "nMsgQueue": "m_nMsgQueue", "m_nMsgQueue": "QUEUE_CAPACITY",
            "enabled": "m_enabled", "m_enabled": "false",
            "position": "m_position", "m_position": "Vector3d(1, 2, 3)",
        }
        constants = {"QUEUE_CAPACITY": "1 << 10"}
        self.assertEqual(generator.initializer_literal("nMsgQueue", constants, initializers.get), 1024)
        self.assertIs(generator.initializer_literal("enabled", constants, initializers.get), False)
        self.assertEqual(generator.initializer_literal("position", constants, initializers.get), [1, 2, 3])

    def test_unknown_dynamic_and_cyclic_initializers_remain_unresolved(self):
        initializers = {
            "dynamic": "readDeviceDefault()", "missing": "m_uninitialized",
            "self": "self", "first": "second", "second": "first",
        }
        for expression in initializers:
            with self.subTest(expression=expression):
                self.assertIs(generator.initializer_literal(expression, {}, initializers.get), generator.MISSING)


class StreamReferenceSchemaTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.schema = generator.generate()
        cls.classes = {entry["name"]: entry for entry in cls.schema["classes"]}

    def parameters(self, name):
        return {tuple(p["path"]): p for p in self.classes[name]["parameters"]}

    def dependencies(self, name):
        return {tuple(d["path"]): d for d in self.classes[name]["dependencies"]}

    def test_rigid_transform_composition(self):
        self.assertNotIn("_TransformBase", self.classes)
        self.assertTrue(self.classes["_TransformRigid"]["creatable"])
        self.assertEqual(self.classes["_TransformRigid"]["buildConditions"], [[]])
        tree = self.dependencies("_TransformRigid")
        self.assertEqual(tree[("_TransformRigidParent",)]["targetClass"], "_TransformRigid")
        self.assertEqual(tree[("_TransformRigidChildren",)]["targetClass"], "_TransformRigid")
        self.assertTrue(tree[("_TransformRigidChildren",)]["multiple"])
        for name in ("_Livox2", "_Airy", "_E1R", "_PCLfile", "_PCmerge", "_PCrecv", "_PCsend",
                     "_PCcrop", "_PCremove", "_PCdownSample", "_PCregistCol", "_OctreeBase",
                     "_GeometryViewerBase", "_NavBase", "_AutopilotBase", "_LCalign"):
            with self.subTest(class_name=name):
                self.assertEqual(self.classes[name]["baseClasses"], ["_ModuleBase"])
                self.assertEqual(self.dependencies(name)[("_TransformRigid",)]["targetClass"], "_TransformRigid")
                for key in ("vT", "vR", "vPos", "vOrt"):
                    self.assertNotIn((key,), self.parameters(name))
        for key in ("vT", "vR"):
            self.assertIn((key,), self.parameters("_TransformRigid"))

    def test_all_declared_factory_classes_are_creatable(self):
        registered = set()
        for filename in ("Modules.cpp", "DataObjects.cpp"):
            source = (generator.ROOT / "src" / "Instance" / filename).read_text()
            registered.update(re.findall(r"^\s*ADD_(?:MODULE|DATA_STREAM)\(\s*(\w+)\s*\)", source, re.M))
        self.assertEqual(
            {name for name, entry in self.classes.items() if entry["creatable"]},
            registered.intersection(self.classes),
        )
        self.assertEqual(set(self.schema["audit"]["factoryClassesWithoutDeclaration"]), registered.difference(self.classes))

    def test_data_object_factories_and_configuration_defaults(self):
        names = {"BBoxStream", "BytePacketStream", "CANframeStream", "IMUstream", "LineFrame", "MavlinkStream", "PCLframe", "PCLmap",
                 "RGBframe", "RGBDframe", "SharedMemoryFrame", "UGLIDcellStream"}
        stream_classes = {"BBoxStream", "BytePacketStream", "CANframeStream", "UGLIDcellStream"}
        guarded_classes = {"RGBframe": "USE_OPENCV", "RGBDframe": "USE_OPENCV", "MavlinkStream": "USE_MAVLINK"}
        self.assertEqual(
            {name for name, entry in self.classes.items() if entry["category"] == "DataObject" and entry["creatable"]},
            names,
        )
        for name in names:
            with self.subTest(class_name=name):
                record, parameters = self.classes[name], self.parameters(name)
                base = "DataObjStream" if name in stream_classes else "DataObjBase"
                self.assertEqual(record["baseClasses"], [base])
                self.assertEqual(record["instanceKind"], "dataObject")
                self.assertEqual(parameters[("type",)]["default"], "dataObject")
                self.assertEqual(parameters[("type",)]["type"], "string")
                self.assertIs(parameters[("bON",)]["default"], True)
                self.assertIs(parameters[("bLog",)]["default"], False)
                guards = [["ifdef " + guarded_classes[name]]] if name in guarded_classes else [[]]
                self.assertEqual(record["buildConditions"], guards)
        self.assertFalse(self.classes["DataObjBase"]["creatable"])
        self.assertFalse(self.classes["DataObjStream"]["creatable"])
        self.assertEqual(self.classes["DataObjStream"]["baseClasses"], ["DataObjBase"])
        self.assertEqual(self.parameters("DataObjStream"), self.parameters("DataObjBase"))
        for name in stream_classes:
            with self.subTest(stream_class=name):
                parameters = self.parameters(name)
                for path, parameter in self.parameters("DataObjBase").items():
                    self.assertEqual(parameters[path], parameter)
                self.assertNotIn(("thread",), {tuple(c["path"]) for c in self.classes[name]["containers"]})
        for name in ("BBoxStream", "CANframeStream", "IMUstream", "UGLIDcellStream"):
            self.assertEqual(self.parameters(name)[("nBuf",)]["type"], "integer")
            self.assertEqual(self.parameters(name)[("nBuf",)]["default"], 1000)
        self.assertEqual(self.parameters("BBoxStream")[("vContainerDim",)]["type"], "array")
        shared = self.parameters("SharedMemoryFrame")
        for key, value in {"shmName": "", "nB": 0, "bWriter": True}.items():
            self.assertEqual(shared[(key,)]["default"], value)

    def test_byte_packet_ports_replace_transport_dependencies(self):
        classes = ("_IObase", "_UDP", "_TCPclient", "_SerialPort", "_WebSocketServer",
                   "_ProtocolBase", "_JSONbase", "_Mavlink", "_USR_CANET", "_RTCMcast")
        for name in classes:
            with self.subTest(class_name=name):
                dependencies = self.dependencies(name)
                for key in ("BytePacketStreamIn", "BytePacketStreamOut"):
                    self.assertEqual(dependencies[(key,)]["targetKind"], "dataObject")
                    self.assertEqual(dependencies[(key,)]["targetClass"], "BytePacketStream")
                self.assertNotIn(("_IObase",), dependencies)
                self.assertNotIn(("_IObaseSend",), dependencies)
        parameters = self.parameters("BytePacketStream")
        self.assertEqual(parameters[("nPacket",)]["default"], 256)
        self.assertEqual(parameters[("nPbuf",)]["default"], 2000)

    def test_mavlink_uses_a_duplex_data_object(self):
        for name in ("_Mavlink", "_APmav_base", "_APmav_copter", "_APmav_rover", "_APmav_RTCM"):
            with self.subTest(class_name=name):
                dependencies = self.dependencies(name)
                stream = dependencies[("MavlinkStream",)]
                self.assertEqual(stream["targetKind"], "dataObject")
                self.assertEqual(stream["targetClass"], "MavlinkStream")
                self.assertTrue(stream["required"])
                self.assertNotIn(("MavlinkStreamIn",), dependencies)
                self.assertNotIn(("MavlinkStreamOut",), dependencies)
                self.assertNotIn(("_Mavlink",), dependencies)
        self.assertNotIn(("vRoutings",), self.dependencies("_Mavlink"))
        queue = self.parameters("MavlinkStream")[("nMsgQueue",)]
        self.assertEqual(queue["type"], "integer")
        self.assertEqual(queue["cppType"], "size_t")
        self.assertEqual(queue["default"], 1024)
        self.assertNotIn("defaultExpression", queue)

    def test_io_threads_are_independently_configurable(self):
        classes = ("_IObase", "_UDP", "_TCPclient", "_SerialPort", "_WebSocket", "_WebSocketServer",
                   "_SocketCAN", "_USR_CANET")
        for name in classes:
            with self.subTest(class_name=name):
                parameters = self.parameters(name)
                containers = {tuple(c["path"]): c["type"] for c in self.classes[name]["containers"]}
                for thread in ("thread", "threadR"):
                    self.assertEqual(containers[(thread,)], "object")
                    self.assertEqual(parameters[(thread, "FPS")]["type"], "number")
                    self.assertEqual(parameters[(thread, "FPS")]["default"], 30)
                    self.assertEqual(parameters[(thread, "bLog")]["type"], "boolean")
                    self.assertNotIn((thread, "class"), parameters)
                    self.assertNotIn((thread, "name"), parameters)

    def test_can_transports_use_frame_streams(self):
        self.assertNotIn("_CANbase", self.classes)
        for name in ("_SocketCAN", "_USR_CANET"):
            with self.subTest(class_name=name):
                self.assertEqual(self.classes[name]["baseClasses"], ["_ModuleBase"])
                for key in ("CANframeStreamIn", "CANframeStreamOut"):
                    dependency = self.dependencies(name)[(key,)]
                    self.assertEqual(dependency["targetKind"], "dataObject")
                    self.assertEqual(dependency["targetClass"], "CANframeStream")

    def test_realsense_validated_configuration_and_option_domains(self):
        parameters = self.parameters("_RealSense")
        expected = {
            "SN": "string", "devFPS": "integer", "devFPSd": "integer",
            "accelFPS": "integer", "gyroFPS": "integer", "tOutMs": "integer",
            "vSizeRGB": "array", "vSizeD": "array", "vRangeD": "array", "dOfs": "number",
            "bRGB": "boolean", "bDepth": "boolean", "bIR": "boolean", "bIMU": "boolean",
            "bPCL": "boolean", "bPCLrgb": "boolean", "bAlign": "boolean", "bDecimation": "boolean",
            "bSpatial": "boolean", "bTemporal": "boolean", "bHoleFilling": "boolean", "bThreshold": "boolean",
            "sensorOptions": "object",
        }
        for key, kind in expected.items():
            with self.subTest(key=key):
                self.assertEqual(parameters[(key,)]["type"], kind)
        for key in ("devURI", "dScale", "btRGB", "btDepth", "bConfidence", "fConfidenceThr"):
            self.assertNotIn((key,), parameters)
        for key, value in {"accelFPS": 0, "gyroFPS": 0, "tOutMs": 1000, "bAlign": False,
                           "vSizeRGB": [640, 480], "vSizeD": [640, 480]}.items():
            self.assertEqual(parameters[(key,)]["default"], value)
        containers = {tuple(c["path"]): c["type"] for c in self.classes["_RealSense"]["containers"]}
        self.assertEqual(containers[("sensorOptions",)], "object")
        for domain in ("depth", "color", "motion", "decimation", "spatial", "temporal", "holeFilling", "threshold"):
            with self.subTest(domain=domain):
                self.assertEqual(containers[("sensorOptions", domain)], "object")
                option = parameters[("sensorOptions", domain, "*")]
                self.assertEqual(option["type"], "json")
                self.assertTrue(option["nullable"])
                self.assertNotIn("default", option)

    def test_glim_nested_parameters_preserve_external_defaults(self):
        parameters = self.parameters("_GLIM")
        expected = {
            ("bMapping",): "boolean", ("nMinPoints",): "integer",
            ("preprocess", "distanceNear"): "number", ("preprocess", "distanceFar"): "number",
            ("preprocess", "voxelResolution"): "number", ("preprocess", "targetPoints"): "integer",
            ("preprocess", "kNeighbors"): "integer", ("preprocess", "threads"): "integer",
            ("odometry", "voxelResolution"): "number", ("odometry", "iterations"): "integer",
            ("odometry", "threads"): "integer", ("submap", "keyframes"): "integer",
            ("submap", "keyframeStrategy"): "string", ("submap", "keyframeTranslation"): "number",
            ("submap", "keyframeRotation"): "number", ("submap", "maxOverlap"): "number",
            ("submap", "voxelResolution"): "number", ("global", "voxelResolution"): "number",
            ("global", "loopDistance"): "number", ("global", "loopOverlap"): "number",
        }
        self.assertEqual(parameters[("parameters",)]["type"], "object")
        for path, kind in expected.items():
            with self.subTest(path=path):
                self.assertEqual(parameters[("parameters",) + path]["type"], kind)
        for path, parameter in parameters.items():
            if path[0] == "parameters":
                self.assertNotIn("default", parameter, path)

    def test_imu_producers_use_stream_ports_and_embedded_threads(self):
        self.assertNotIn("_IMUbase", self.classes)
        producers = {"_Airy": ("thread",), "_Livox2": ("thread",),
                     "_RealSense": ("thread", "threadPP"), "_Orbbec": ("thread", "threadPP")}
        for name, threads in producers.items():
            with self.subTest(class_name=name):
                parameters = self.parameters(name)
                dependencies = self.dependencies(name)
                stream = dependencies[("IMUstreamOut",)]
                self.assertEqual(parameters[("IMUstreamOut",)]["type"], "string")
                self.assertEqual(stream["targetClass"], "IMUstream")
                self.assertEqual(stream["targetKind"], "dataObject")
                self.assertFalse(stream["multiple"])
                self.assertFalse(stream.get("required", False))
                self.assertNotIn(("_IMUbase",), dependencies)
                containers = {tuple(c["path"]): c["type"] for c in self.classes[name]["containers"]}
                self.assertNotIn(("threadStream",), containers)
                self.assertFalse(any(p[0] == "threadStream" for p in parameters))
                for thread in threads:
                    self.assertEqual(containers[(thread,)], "object")
                    self.assertEqual(parameters[(thread, "FPS")]["type"], "number")
                    self.assertEqual(parameters[(thread, "bLog")]["type"], "boolean")
                    self.assertIs(parameters[(thread, "bLog")]["default"], False)
                    self.assertNotIn((thread, "class"), parameters)
                    self.assertNotIn((thread, "name"), parameters)

    def test_optional_stream_links_do_not_block_valid_configurations(self):
        optional = {
            "_Camera": ("RGBframeOut",),
            "_RealSense": ("RGBframeOut", "RGBDframeOut", "RGBDtRGBframeOut", "RGBDtDframeOut",
                           "DframeOut", "IRframeOut", "PCLframeOut", "IMUstreamOut"),
            "_GLIM": ("PCLframeOut", "PCLmapOut", "IMUstreamIn"),
        }
        for name, keys in optional.items():
            for key in keys:
                with self.subTest(class_name=name, key=key):
                    self.assertFalse(self.dependencies(name)[(key,)].get("required", False))
        for name, key in (("_GLIM", "PCLframeIn"), ("_Crop", "RGBframeIn"), ("_WebGLIM", "PCLmapIn")):
            self.assertTrue(self.dependencies(name)[(key,)]["required"])

    def test_dependencies_identify_the_runtime_factory(self):
        for record in self.classes.values():
            if record["creatable"] and record["category"] != "DataObject":
                self.assertEqual(record["instanceKind"], "module", record["name"])
            for dependency in record["dependencies"]:
                with self.subTest(class_name=record["name"], path=dependency["path"]):
                    self.assertIn(dependency["targetKind"], ("module", "dataObject"))
        self.assertEqual(self.dependencies("_Console")[("vBASE",)]["targetKind"], "module")
        for name, path in (("_Camera", ("RGBframeOut",)), ("_GLIM", ("PCLmapOut",)),
                           ("_PCmerge", ("vPCLframesIn",)), ("_WebGeometry", ("vGeometry", "*", "PCLframeIn"))):
            self.assertEqual(self.dependencies(name)[path]["targetKind"], "dataObject")

    def test_detection_and_tracking_references_are_name_strings(self):
        classes = self.classes
        expected = {
            "_Contour": ["RGBframeIn", "BBoxStreamOut"],
            "_ArUco": ["RGBframeIn", "BBoxStreamOut"],
            "_YOLO26detectONNX": ["RGBframeIn", "BBoxStreamOut"],
            "_SingleTracker": ["RGBframeIn", "BBoxStreamOut"],
            "_APmav_follow": ["BBoxStreamIn", "BBoxStreamTrackIn"],
            "_APmav_land": ["BBoxStreamIn", "BBoxStreamTrackIn"],
        }
        for name, keys in expected.items():
            parameters = {tuple(p["path"]): p for p in classes[name]["parameters"]}
            dependencies = {tuple(d["path"]): d for d in classes[name]["dependencies"]}
            for key in keys:
                with self.subTest(class_name=name, key=key):
                    parameter = parameters[(key,)]
                    dependency = dependencies[(key,)]
                    self.assertEqual(parameter["type"], "string")
                    self.assertTrue(parameter["dependency"])
                    self.assertFalse(dependency["multiple"])
                    target = "RGBframe" if key == "RGBframeIn" else "BBoxStream"
                    self.assertEqual(dependency["targetClass"], target)


    def test_directional_data_object_references(self):
        expected = {
            "_Camera": {"RGBframeOut": "RGBframe"},
            "_YOLO26depthEstONNX": {
                "RGBframeIn": "RGBframe", "RGBDframeOut": "RGBDframe",
                "DframeOut": "RGBframe", "PCLframeOut": "PCLframe",
            },
            "_Crop": {"RGBframeIn": "RGBframe", "RGBframeOut": "RGBframe"},
            "_Mask": {"RGBframeMaskIn": "RGBframe"},
            "_Orbbec": {
                "RGBframeOut": "RGBframe", "DframeOut": "RGBframe", "IRframeOut": "RGBframe",
                "RGBDframeOut": "RGBDframe", "RGBDtRGBframeOut": "RGBDframe",
                "RGBDtDframeOut": "RGBDframe", "PCLframeOut": "PCLframe", "IMUstreamOut": "IMUstream",
            },
            "_Livox2": {"PCLframeOut": "PCLframe", "IMUstreamOut": "IMUstream"},
            "_GLIM": {"PCLframeIn": "PCLframe", "IMUstreamIn": "IMUstream", "PCLframeOut": "PCLframe", "PCLmapOut": "PCLmap"},
            "_WebGLIM": {"PCLmapIn": "PCLmap"},
            "_PCregistICP": {"PCLframeSrcIn": "PCLframe", "PCLframeTgtIn": "PCLframe"},
        }
        for name, keys in expected.items():
            parameters = {tuple(p["path"]): p for p in self.classes[name]["parameters"]}
            dependencies = {tuple(d["path"]): d for d in self.classes[name]["dependencies"]}
            for key, target in keys.items():
                with self.subTest(class_name=name, key=key):
                    self.assertEqual(parameters[(key,)]["type"], "string")
                    self.assertTrue(parameters[(key,)]["dependency"])
                    self.assertEqual(dependencies[(key,)]["targetClass"], target)
                    self.assertFalse(dependencies[(key,)]["multiple"])
            for old in ("RGBframe", "RGBframeMask", "Dframe", "IRframe", "RGBDframe", "RGBDtRGBframe", "RGBDtDframe", "PCLframe", "PCLmap", "IMUframe", "IMUstream", "PCLframeSrc", "PCLframeTgt"):
                self.assertNotIn((old,), dependencies, name)

    def test_depth_detector_link_override(self):
        record = self.classes['_YOLO26depthEstONNX']
        parameters = {tuple(p['path']): p for p in record['parameters']}
        dependencies = {tuple(d['path']): d for d in record['dependencies']}
        self.assertNotIn(('BBoxStreamOut',), parameters)
        self.assertNotIn(('BBoxStreamOut',), dependencies)
        self.assertTrue(dependencies[('RGBframeIn',)]['required'])
        self.assertTrue(dependencies[('RGBDframeOut',)]['required'])
        self.assertFalse(dependencies[('DframeOut',)].get('required', False))
        self.assertFalse(dependencies[('PCLframeOut',)].get('required', False))

    def test_directional_list_and_viewer_adapters(self):
        for name in ("_OctreeGrid", "_SelectableOctGrid", "_PCmerge"):
            dependencies = {tuple(d["path"]): d for d in self.classes[name]["dependencies"]}
            with self.subTest(class_name=name):
                self.assertEqual(dependencies[("vPCLframesIn",)]["targetClass"], "PCLframe")
                self.assertTrue(dependencies[("vPCLframesIn",)]["multiple"])
                self.assertNotIn(("vPCLframes",), dependencies)
        for name in ("_WebGeometry", "_WebSelectableOctGrid", "_ImGUIselectableOctGrid"):
            dependencies = {tuple(d["path"]): d for d in self.classes[name]["dependencies"]}
            for target in ("PCLframe", "LineFrame"):
                with self.subTest(class_name=name, target=target):
                    self.assertEqual(dependencies[("vGeometry", "*", target + "In")]["targetClass"], target)
                    self.assertNotIn(("vGeometry", "*", target), dependencies)


if __name__ == "__main__":
    unittest.main()
