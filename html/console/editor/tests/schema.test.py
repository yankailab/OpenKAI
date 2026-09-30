#!/usr/bin/env python3
"""Run with python3 html/console/editor/tests/schema.test.py."""
import importlib.util
from pathlib import Path
import unittest


generator_path = Path(__file__).resolve().parents[1] / "tools" / "generate-schema.py"
spec = importlib.util.spec_from_file_location("openkai_schema", generator_path)
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


class StreamReferenceSchemaTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.classes = {entry["name"]: entry for entry in generator.generate()["classes"]}

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
