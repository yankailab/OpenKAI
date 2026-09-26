/* Run with node --test tests/model.test.js, or call runOpenKAIModelTests() in a browser. */
(function (root) {
    "use strict";
    const schema = { classes: [
        { name: "BASE", creatable: false, baseClasses: [], parameters: [], dependencies: [] },
        { name: "ModuleMgr", creatable: false, baseClasses: [], parameters: [], dependencies: [] },
        { name: "_VisionBase", creatable: false, baseClasses: ["BASE"], parameters: [], dependencies: [] },
        { name: "Camera", creatable: true, baseClasses: ["_VisionBase"], parameters: [{ path: ["FPS"], type: "number", default: 30 }, { path: ["bON"], type: "boolean", default: true }], dependencies: [] },
        { name: "Serial", creatable: true, baseClasses: ["BASE"], parameters: [], dependencies: [] },
        { name: "Consumer", creatable: true, baseClasses: ["BASE"], parameters: [{ path: ["gain"], type: "number" }], dependencies: [
            { path: ["input"], targetClass: "_VisionBase", multiple: false, required: true },
            { path: ["inputs"], targetClass: "BASE", multiple: true },
            { path: ["rows", "*", "source"], targetClass: "_VisionBase", multiple: false, containers: [{ path: ["rows"], type: "array" }] },
            { path: ["states", "*", "source"], targetClass: "BASE", multiple: false, containers: [{ path: ["states"], type: "object" }] },
            { path: ["groups", "*", "members", "*", "source"], targetClass: "BASE", multiple: false, containers: [{ path: ["groups", "*", "members"], type: "object" }, { path: ["groups"], type: "array" }] },
        ] },
    ] };

    function suite(Model, test, assert) {
        test("import preserves unknown classes, includes, extension data, nested values and comment-like strings", () => {
            const doc = { APP: { class: "ModuleMgr", vInclude: ["another.json"] }, mysterious: { class: "Custom", object: { empty: [], value: null, text: "http://host/*literal*/" } }, number: 5 };
            const model = new Model(schema, doc);
            assert.deepEqual(model.toJSON(), doc);
            assert.deepEqual(JSON.parse(model.export()), doc);
            assert.equal(model.nodes().length, 2);
            assert.ok(model.validate().some(d => d.code === "unknown-class"));
            model.setValue("/mysterious", ["object", "value"], true);
            assert.equal(doc.mysterious.object.value, null, "import must not mutate the caller's object");
            assert.deepEqual(model.toJSON().APP, doc.APP);
        });
        test("block comments import without modifying string contents; failed imports are atomic", () => {
            const model = new Model(schema, '/* config */ {"camera": {"class":"Camera", "pipeline":"http://x/*not a comment*/"}}');
            const before = model.export();
            assert.equal(model.getValue("/camera", ["pipeline"]), "http://x/*not a comment*/");
            assert.throws(() => model.import('{"bad":}'));
            assert.throws(() => model.import("[]"));
            assert.throws(() => model.import("/*"));
            assert.equal(model.export(), before);
        });
        test("recursive class discovery uses collision-free JSON pointers", () => {
            const model = new Model(schema, { "a/b~c": { class: "Camera", options: { worker: { class: "Consumer", input: "a/b~c" } } } });
            const nodes = model.nodes();
            assert.equal(nodes[0].id, "/a~1b~0c");
            assert.equal(nodes[1].parentId, nodes[0].id);
            assert.equal(nodes[1].nested, true);
            assert.equal(model.dependencies()[0].targetId, nodes[0].id);
        });
        test("dependency resolution follows exact runtime name overrides, without treating slash as root syntax", () => {
            const model = new Model(schema, {
                APP: { class: "ModuleMgr" }, camera: { class: "Camera", name: "capture" },
                consumer: { class: "Consumer", input: "capture", inputs: ["camera", "/capture", "APP"] },
            });
            assert.equal(model.resolveReference("capture").id, "/camera");
            assert.equal(model.resolveReference("camera"), null);
            assert.equal(model.resolveReference("/capture"), null);
            assert.equal(model.resolveReference("APP"), null);
            assert.deepEqual(model.dependencies().map(edge => edge.resolved), [true, false, false, false]);
        });
        test("compatible targets include inherited classes and reject unrelated or nested modules", () => {
            const model = new Model(schema, { camera: { class: "Camera", nested: { class: "Camera" } }, serial: { class: "Serial" }, consumer: { class: "Consumer" }, APP: { class: "ModuleMgr" } });
            model.connect("/consumer", ["input"], "/camera");
            assert.equal(model.getValue("/consumer", ["input"]), "camera");
            assert.throws(() => model.connect("/consumer", ["input"], "/serial"));
            assert.throws(() => model.connect("/consumer", ["input"], "/camera/nested"));
            assert.throws(() => model.connect("/consumer", ["inputs"], "/APP"));
            assert.equal(model.getValue("/consumer", ["input"]), "camera");
        });
        test("array dependency edits preserve sibling metadata and disconnect only selected entries", () => {
            const model = new Model(schema, { a: { class: "Camera" }, b: { class: "Camera" }, consumer: { class: "Consumer", rows: [{ source: "a", color: [1, 2, 3] }], inputs: ["a"] } });
            model.connect("/consumer", ["rows", "*", "source"], "/b");
            assert.deepEqual(model.getValue("/consumer", ["rows"]), [{ source: "a", color: [1, 2, 3] }, { source: "b" }]);
            model.connect("/consumer", ["rows", "0", "source"], "/b");
            model.connect("/consumer", ["inputs"], "/b");
            model.connect("/consumer", ["inputs"], "/b");
            assert.deepEqual(model.getValue("/consumer", ["inputs"]), ["a", "b"]);
            model.disconnect("/consumer", ["inputs", "0"]);
            model.disconnect("/consumer", ["rows", "0", "source"]);
            assert.deepEqual(model.getValue("/consumer", ["inputs"]), ["b"]);
            assert.deepEqual(model.getValue("/consumer", ["rows", "0"]), { color: [1, 2, 3] });
        });
        test("object wildcard dependencies require concrete keys, never overwrite maps with arrays", () => {
            const model = new Model(schema, { camera: { class: "Camera" }, consumer: { class: "Consumer", states: { idle: { source: "camera", duration: 1 } } } });
            const before = model.export();
            assert.throws(() => model.connect("/consumer", ["states", "*", "source"], "/camera"));
            assert.equal(model.export(), before);
            model.connect("/consumer", ["states", "run", "source"], "/camera");
            assert.deepEqual(model.getValue("/consumer", ["states", "idle"]), { source: "camera", duration: 1 });
            assert.equal(model.getValue("/consumer", ["states", "run", "source"]), "camera");
        });
        test("numeric map keys create objects and preserve other map rows", () => {
            const model = new Model(schema, { a: { class: "Camera" }, b: { class: "Camera" }, consumer: { class: "Consumer" } });
            model.connect("/consumer", ["states", "0", "source"], "/a");
            assert.deepEqual(model.getValue("/consumer", ["states"]), { "0": { source: "a" } });
            model.setValue("/consumer", ["states", "0", "duration"], 5);
            model.connect("/consumer", ["states", "4", "source"], "/b");
            assert.deepEqual(model.getValue("/consumer", ["states"]), { "0": { source: "a", duration: 5 }, "4": { source: "b" } });
            model.connect("/consumer", ["states", "0", "source"], "/b");
            assert.deepEqual(model.getValue("/consumer", ["states"]), { "0": { source: "b", duration: 5 }, "4": { source: "b" } });
        });
        test("nested wildcard container metadata creates numeric object keys within array rows", () => {
            const model = new Model(schema, { camera: { class: "Camera" }, consumer: { class: "Consumer" } });
            model.connect("/consumer", ["groups", "*", "members", "9", "source"], "/camera");
            assert.deepEqual(model.getValue("/consumer", ["groups"]), [{ members: { "9": { source: "camera" } } }]);
            model.connect("/consumer", ["groups", "0", "members", "2", "source"], "/camera");
            assert.deepEqual(model.getValue("/consumer", ["groups"]), [{ members: { "2": { source: "camera" }, "9": { source: "camera" } } }]);
            model.connect("/consumer", ["groups", "*", "members", "0", "source"], "/camera");
            assert.equal(model.getValue("/consumer", ["groups"]).length, 2);
            const before = model.export();
            assert.throws(() => model.connect("/consumer", ["groups", "0", "members", "*", "source"], "/camera"));
            assert.equal(model.export(), before);
        });
        test("renaming updates only resolvable references, including arrays and nested referring nodes", () => {
            const model = new Model(schema, {
                camera: { class: "Camera", name: "capture", nested: { class: "Consumer", input: "capture" } },
                consumer: { class: "Consumer", input: "capture", inputs: ["capture", "/capture"], rows: [{ source: "capture", label: "capture" }], unknown: "capture" },
            });
            const renamed = model.renameNode("/camera", "newCamera");
            assert.equal(renamed.id, "/newCamera");
            assert.equal(renamed.name, "newCamera");
            assert.equal(model.getValue("/newCamera/nested", ["input"]), "newCamera");
            assert.equal(model.getValue("/consumer", ["input"]), "newCamera");
            assert.deepEqual(model.getValue("/consumer", ["inputs"]), ["newCamera", "/capture"]);
            assert.deepEqual(model.getValue("/consumer", ["rows"]), [{ source: "newCamera", label: "capture" }]);
            assert.equal(model.getValue("/consumer", ["unknown"]), "capture");
        });
        test("deleting a module leaves unresolved references and unrelated configuration intact", () => {
            const model = new Model(schema, { camera: { class: "Camera" }, consumer: { class: "Consumer", input: "camera", unknown: [1, 2] } });
            model.removeNode("/camera");
            assert.deepEqual(model.toJSON(), { consumer: { class: "Consumer", input: "camera", unknown: [1, 2] } });
            assert.equal(model.dependencies()[0].resolved, false);
            assert.ok(model.validate().some(d => d.code === "unresolved-dependency"));
        });
        test("new modules use literal defaults, unique names and only registered classes", () => {
            const model = new Model(schema);
            assert.equal(model.addNode("Camera").id, "/camera");
            assert.equal(model.addNode("Camera").id, "/camera2");
            assert.equal(model.getValue("/camera", ["FPS"]), 30);
            assert.equal(model.getValue("/camera", ["bON"]), true);
            assert.equal(model.addNode("ModuleMgr").id, "/APP");
            assert.throws(() => model.addNode("BASE"));
            assert.throws(() => model.addNode("Camera", "camera"));
            assert.throws(() => model.renameNode("/camera", "camera2"));
        });
        test("adding, renaming and connecting cannot introduce ambiguous runtime names", () => {
            const model = new Model(schema, { alias: { class: "Camera", name: "camera" }, other: { class: "Camera" }, consumer: { class: "Consumer" } });
            assert.equal(model.addNode("Camera").key, "camera2");
            assert.throws(() => model.addNode("Camera", "camera"));
            assert.throws(() => model.renameNode("/other", "camera"));
            model.setValue("/other", ["name"], "camera");
            assert.throws(() => model.connect("/consumer", ["input"], "/alias"));
            assert.ok(model.validate().some(d => d.code === "duplicate-name"));
        });
        test("validation surfaces missing, malformed, disabled and incompatible dependencies", () => {
            const model = new Model(schema, {
                camera: { class: "Camera", bON: false }, serial: { class: "Serial" },
                a: { class: "Consumer" }, b: { class: "Consumer", input: "serial", gain: "bad" },
                c: { class: "Consumer", input: "camera", inputs: [42] },
            });
            const codes = model.validate().map(d => d.code);
            ["missing-dependency", "parameter-type", "incompatible-dependency", "disabled-dependency", "dependency-type"].forEach(code => assert.ok(codes.includes(code), code));
        });
        test("boolean bON controls disabled dependencies and preserves imported values", () => {
            const doc = {
                disabled: { class: "Camera", bON: false },
                enabled: { class: "Camera", bON: true },
                defaultEnabled: { class: "Camera" },
                legacyDisabled: { class: "Camera", bON: 0 },
                legacyEnabled: { class: "Camera", bON: 1 },
                consumer: { class: "Consumer", inputs: ["disabled", "enabled", "defaultEnabled", "legacyDisabled", "legacyEnabled"] },
            };
            const model = new Model(schema, doc);
            assert.deepEqual(JSON.parse(model.export()), doc);
            assert.deepEqual(model.dependencies().map(edge => edge.targetDisabled), [true, false, false, false, false]);
            assert.equal(model.validate().filter(d => d.code === "disabled-dependency").length, 1);
            assert.deepEqual(model.validate().filter(d => d.code === "parameter-type").map(d => d.nodeId), ["/legacyDisabled", "/legacyEnabled"]);
            model.setValue("/disabled", ["bON"], true);
            assert.equal(model.validate().filter(d => d.code === "disabled-dependency").length, 0);
            model.deleteValue("/disabled", ["bON"]);
            assert.equal(model.dependencies()[0].targetDisabled, false);
            assert.equal(Object.hasOwn(JSON.parse(model.export()).disabled, "bON"), false);
        });
        test("unsafe JSON keys remain inert own data through import, edit, export and rename", () => {
            const model = new Model(schema, '{"__proto__":{"class":"Camera","polluted":true},"consumer":{"class":"Consumer","input":"__proto__","constructor":{"prototype":{"local":1}}}}');
            model.setValue("/consumer", ["__proto__", "injected"], true);
            model.setValue("/consumer", ["constructor", "prototype", "value"], 9);
            assert.equal({}.injected, undefined);
            assert.equal({}.polluted, undefined);
            assert.equal(Object.prototype.value, undefined);
            assert.equal(model.getValue("/consumer", ["__proto__", "injected"]), true);
            assert.equal(model.resolveReference("__proto__").id, "/__proto__");
            model.renameNode("/__proto__", "constructor");
            assert.equal(model.getValue("/consumer", ["input"]), "constructor");
            assert.equal(Object.prototype.polluted, undefined);
            assert.equal(JSON.parse(model.export()).consumer.constructor.prototype.local, 1);
        });
        test("failed path edits and connections preserve all existing data", () => {
            const model = new Model(schema, { camera: { class: "Camera" }, consumer: { class: "Consumer", scalar: 2, inputs: "malformed" } });
            const before = model.export();
            assert.throws(() => model.setValue("/consumer", ["scalar", "child"], 2));
            assert.throws(() => model.connect("/consumer", ["inputs"], "/camera"));
            assert.throws(() => model.setValue("/consumer", ["class"], 1));
            assert.throws(() => model.deleteValue("/consumer", ["class"]));
            assert.throws(() => model.setValue("/consumer", ["newArray", "2", "value"], 1));
            assert.equal(model.export(), before);
        });
    }

    if (typeof module === "object" && module.exports) {
        const Model = require("../js/model.js");
        const test = require("node:test"), assert = require("node:assert/strict");
        const fs = require("node:fs"), path = require("node:path");
        suite(Model, test, assert);
        test("generated catalog declares boolean bON defaults for modules and embedded classes", () => {
            const catalog = JSON.parse(fs.readFileSync(path.join(__dirname, "../OpenKAI.json"), "utf8"));
            let switches = 0;
            for (const definition of catalog.classes) {
                for (const parameter of definition.parameters || []) {
                    if (parameter.path.at(-1) !== "bON") continue;
                    switches++;
                    assert.equal(parameter.type, "boolean", definition.name + ": " + parameter.path.join("."));
                    assert.equal(parameter.default, true, definition.name + ": " + parameter.path.join("."));
                }
            }
            assert.ok(switches > 0);
            const model = new Model(catalog);
            const camera = model.addNode("_Camera");
            assert.equal(camera.data.bON, true);
            model.setValue(camera.id, ["bON"], false);
            assert.equal(JSON.parse(model.export())[camera.key].bON, false);
            assert.equal(model.validate().some(d => d.code === "parameter-type" && d.path.at(-1) === "bON"), false);
        });
        test("every repository jsonCfg document round-trips without dropping fields", () => {
            const repo = path.resolve(__dirname, "../../../..");
            const catalogPath = path.join(__dirname, "../OpenKAI.json");
            const catalog = fs.existsSync(catalogPath) ? JSON.parse(fs.readFileSync(catalogPath, "utf8")) : schema;
            function files(directory) {
                return fs.readdirSync(directory, { withFileTypes: true }).flatMap(entry => entry.isDirectory() ? files(path.join(directory, entry.name)) : entry.name.endsWith(".json") ? [path.join(directory, entry.name)] : []);
            }
            for (const file of files(path.join(repo, "jsonCfg"))) {
                const text = fs.readFileSync(file, "utf8");
                const expected = JSON.parse(Model.stripComments(text));
                const model = new Model(catalog, text);
                assert.deepEqual(JSON.parse(model.export()), expected, path.basename(file));
                model.dependencies();
                model.validate();
            }
        });
    } else {
        root.runOpenKAIModelTests = function () {
            const results = [];
            const fail = message => { throw new Error(message || "Assertion failed"); };
            const assert = {
                equal: (a, b, message) => { if (a !== b) fail(message || String(a) + " !== " + String(b)); },
                deepEqual: (a, b, message) => { if (JSON.stringify(a) !== JSON.stringify(b)) fail(message || JSON.stringify(a) + " !== " + JSON.stringify(b)); },
                ok: (condition, message) => { if (!condition) fail(message); },
                throws: callback => { let threw = false; try { callback(); } catch (_) { threw = true; } if (!threw) fail("Expected an exception"); },
            };
            suite(root.OpenKAIModel, (name, callback) => {
                try { callback(); results.push({ name, passed: true }); }
                catch (error) { results.push({ name, passed: false, error: error.message, stack: error.stack }); }
            }, assert);
            return results;
        };
    }
})(typeof globalThis !== "undefined" ? globalThis : this);
