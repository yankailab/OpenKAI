"""Regenerate tiny test-only graphs with `python generate.py` (requires onnx).

These are deterministic synthetic graphs, not YOLO weights. Checked-in ONNX files
let the C++ tests run without Python, Ultralytics, a network, or a camera.
"""
from pathlib import Path

import onnx
from onnx import TensorProto, helper

DEST = Path(__file__).resolve().parent


def save(name, nodes, input_shape, output_shape, initializers=()):
    graph = helper.make_graph(
        nodes,
        name,
        [helper.make_tensor_value_info("images", TensorProto.FLOAT, input_shape)],
        [helper.make_tensor_value_info("depth", TensorProto.FLOAT, output_shape)],
        initializer=list(initializers),
    )
    model = helper.make_model(
        graph, opset_imports=[helper.make_opsetid("", 13)], ir_version=8,
        producer_name="OpenKAI synthetic depth tests",
    )
    onnx.checker.check_model(model)
    onnx.save(model, DEST / f"{name}.onnx")


for rank in (2, 3, 4):
    for dynamic in (False, True):
        height, width = ("height", "width") if dynamic else (4, 4)
        dimensions = [1, 1, height, width]
        initializers = [
            helper.make_tensor("weights", TensorProto.FLOAT, [1, 3, 1, 1], [1, 10, 100]),
            helper.make_tensor("bias", TensorProto.FLOAT, [1], [2]),
        ]
        nodes = [helper.make_node("Conv", ["images", "weights", "bias"],
                                  ["depth" if rank == 4 else "dense"])]
        if rank < 4:
            initializers.append(helper.make_tensor("axes", TensorProto.INT64,
                                                   [4 - rank], list(range(4 - rank))))
            nodes.append(helper.make_node("Squeeze", ["dense", "axes"], ["depth"]))
        save(f"channels_{rank}d_{'dynamic' if dynamic else 'static'}", nodes,
             [1, 3, height, width], dimensions[4-rank:], initializers)

invalid = helper.make_tensor("values", TensorProto.FLOAT, [1, 1, 2, 4],
                             [-1, 0, float("nan"), float("inf"), 0.1, 1, 3, 100])
save("invalid_depth", [helper.make_node("Constant", [], ["depth"], value=invalid)],
     [1, 3, 2, 4], [1, 1, 2, 4])
detection = helper.make_tensor("channels", TensorProto.FLOAT, [1, 2, 4, 4], [1] * 32)
save("wrong_output", [helper.make_node("Constant", [], ["depth"], value=detection)],
     [1, 3, 4, 4], [1, 2, 4, 4])

reduced = helper.make_tensor("coarse", TensorProto.FLOAT, [1, 1, 2, 2], [2, 4, 6, 8])
save("reduced_depth", [helper.make_node("Constant", [], ["depth"], value=reduced)],
     [1, 3, 4, 4], [1, 1, 2, 2])
save("padding_mean", [helper.make_node("Conv", ["images", "weights", "bias"], ["dense"]),
                       helper.make_node("GlobalAveragePool", ["dense"], ["depth"])],
     [1, 3, 4, 4], [1, 1, 1, 1], [
         helper.make_tensor("weights", TensorProto.FLOAT, [1, 3, 1, 1], [1, 10, 100]),
         helper.make_tensor("bias", TensorProto.FLOAT, [1], [2]),
     ])
