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
    def test_detection_and_tracking_references_are_name_strings(self):
        classes = {entry["name"]: entry for entry in generator.generate()["classes"]}
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


if __name__ == "__main__":
    unittest.main()
