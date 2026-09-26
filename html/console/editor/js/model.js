/* OpenKAI config model. Works as a classic browser script and a Node module. */
(function (root, factory) {
    "use strict";
    const Model = factory();
    if (typeof module === "object" && module.exports) module.exports = Model;
    else root.OpenKAIModel = Model;
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
    "use strict";

    const own = (object, key) => object != null && Object.prototype.hasOwnProperty.call(object, key);
    const object = value => value !== null && typeof value === "object" && !Array.isArray(value);
    const clone = value => JSON.parse(JSON.stringify(value));
    const pointer = path => "/" + path.map(part => String(part).replace(/~/g, "~0").replace(/\//g, "~1")).join("/");
    const samePath = (a, b) => a.length === b.length && a.every((part, i) => String(part) === String(b[i]));
    const matches = (pattern, path) => pattern.length === path.length && pattern.every((part, i) => part === "*" || String(part) === String(path[i]));
    const numeric = value => /^(0|[1-9][0-9]*)$/.test(String(value));
    const put = (container, key, value) => {
        // Defining own properties keeps imported JSON keys such as __proto__ inert.
        Object.defineProperty(container, key, { value, enumerable: true, writable: true, configurable: true });
    };
    function get(root, path) {
        let current = root;
        for (const key of path) {
            if (!own(current, key)) return undefined;
            current = current[key];
        }
        return current;
    }
    function set(root, path, value) {
        if (!Array.isArray(path) || !path.length || path.includes("*")) throw new Error("A concrete parameter path is required.");
        let current = root;
        for (let i = 0; i < path.length - 1; i++) {
            const key = path[i];
            if (Array.isArray(current) && (!numeric(key) || Number(key) > current.length)) throw new Error("Array indexes must be consecutive nonnegative integers.");
            if (!own(current, key)) put(current, key, numeric(path[i + 1]) ? [] : {});
            if (current[key] === null || typeof current[key] !== "object") throw new Error("Cannot edit inside a scalar parameter: " + path.slice(0, i + 1).join("."));
            current = current[key];
        }
        const key = path[path.length - 1];
        if (Array.isArray(current) && (!numeric(key) || Number(key) > current.length)) throw new Error("Array indexes must be consecutive nonnegative integers.");
        put(current, key, value);
    }
    function erase(root, path) {
        if (!Array.isArray(path) || !path.length || path.includes("*")) throw new Error("A concrete parameter path is required.");
        const parent = get(root, path.slice(0, -1));
        if (parent === undefined || parent === null || typeof parent !== "object") return;
        const key = path[path.length - 1];
        if (Array.isArray(parent) && numeric(key)) parent.splice(Number(key), 1);
        else delete parent[key];
    }
    // Only block comments are supported by JsonCfg. This scanner keeps comment
    // markers inside strings (URLs, pipelines, paths) intact.
    function stripComments(text) {
        let output = "", quoted = false, escaped = false;
        for (let i = 0; i < text.length; i++) {
            const ch = text[i];
            if (quoted) {
                output += ch;
                if (escaped) escaped = false;
                else if (ch === "\\") escaped = true;
                else if (ch === '"') quoted = false;
            } else if (ch === '"') {
                quoted = true;
                output += ch;
            } else if (ch === "/" && text[i + 1] === "*") {
                const end = text.indexOf("*/", i + 2);
                if (end < 0) throw new Error("Unterminated block comment.");
                output += text.slice(i, end + 2).replace(/[^\n\r]/g, " ");
                i = end + 1;
            } else output += ch;
        }
        return output;
    }
    // Expand schema wildcard paths against existing data, never inventing values.
    function expand(root, pattern, includeMissing) {
        const results = [];
        function walk(current, at, path) {
            if (at === pattern.length) {
                results.push({ path, value: current, present: current !== undefined });
                return;
            }
            const key = pattern[at];
            if (key === "*") {
                if (current && typeof current === "object") {
                    Object.keys(current).forEach(child => walk(current[child], at + 1, path.concat(child)));
                }
            } else if (own(current, key)) walk(current[key], at + 1, path.concat(key));
            else if (includeMissing) walk(undefined, at + 1, path.concat(key));
        }
        walk(root, 0, []);
        return results;
    }
    function valueType(value) {
        if (value === null) return "null";
        if (Array.isArray(value)) return "array";
        return typeof value;
    }
    function acceptsType(value, definition) {
        if (value === null && definition.nullable) return true;
        const types = Array.isArray(definition.type) ? definition.type : [definition.type];
        if (!types[0] || types.includes("unknown") || types.includes("any") || types.includes("json")) return true;
        return types.some(type => type === "integer" ? Number.isInteger(value) : type === valueType(value));
    }

    class OpenKAIModel {
        constructor(schema, document) {
            this.schema = schema || { classes: [] };
            this.classes = new Map((this.schema.classes || []).map(definition => [definition.name, definition]));
            this.document = {};
            if (document !== undefined) this.import(document);
        }

        /** Import is atomic. Unknown classes, values, includes and key order survive. */
        import(input) {
            const parsed = typeof input === "string" ? JSON.parse(stripComments(input.replace(/^\uFEFF/, ""))) : clone(input);
            if (!object(parsed)) throw new Error("An OpenKAI configuration must be a JSON object.");
            this.document = parsed;
            return this;
        }

        export(indent = 2) { return JSON.stringify(this.document, null, indent) + "\n"; }
        toJSON() { return clone(this.document); }
        getClass(name) { return this.classes.get(name) || null; }

        /** IDs are JSON pointers into the config, not runtime module references. */
        nodes() {
            const result = [];
            const visit = (value, path, parentId) => {
                if (!value || typeof value !== "object") return;
                let nextParent = parentId;
                if (object(value) && own(value, "class") && typeof value.class === "string") {
                    const key = String(path[path.length - 1] || "");
                    const id = pointer(path);
                    result.push({
                        id, path, key, name: typeof value.name === "string" ? value.name : key,
                        className: value.class, data: value, definition: this.getClass(value.class),
                        parentId, nested: path.length !== 1,
                    });
                    nextParent = id;
                }
                for (const key of Object.keys(value)) visit(value[key], path.concat(key), nextParent);
            };
            for (const key of Object.keys(this.document)) visit(this.document[key], [key], null);
            return result;
        }

        getNode(id) {
            const node = this.nodes().find(item => item.id === id);
            if (!node) throw new Error("Instance no longer exists: " + id);
            return node;
        }

        compatible(className, requiredClass) {
            if (!requiredClass || requiredClass === "void" || requiredClass === "any") return true;
            const wanted = Array.isArray(requiredClass) ? requiredClass : [requiredClass];
            const seen = new Set();
            const visit = name => {
                if (wanted.includes(name)) return true;
                if (seen.has(name)) return false;
                seen.add(name);
                const definition = this.getClass(name);
                return !!definition && (definition.baseClasses || []).some(base => visit(typeof base === "string" ? base : base.name));
            };
            return visit(className);
        }

        /** ModuleMgr::findModule compares exact names; /foo is not an alias for foo. */
        resolveReference(reference) {
            if (typeof reference !== "string" || !reference) return null;
            return this.nodes().filter(node => !node.nested && node.className !== "ModuleMgr")
                .sort((a, b) => a.key < b.key ? -1 : a.key > b.key ? 1 : 0)
                .find(node => node.name === reference) || null;
        }

        addNode(className, name, parentPath = []) {
            const definition = this.getClass(className);
            if (!definition) throw new Error("Unknown class: " + className);
            if (!parentPath.length && definition.creatable === false && className !== "ModuleMgr") {
                throw new Error(className + " is not registered as a top-level module.");
            }
            const parent = get(this.document, parentPath);
            if (!object(parent)) throw new Error("New instances need an object container.");
            const runtimeNames = new Set(parentPath.length ? [] : this.nodes().filter(node => !node.nested).map(node => node.name));
            let key = name || (className === "ModuleMgr" ? "APP" : className.replace(/^_+/, "").replace(/^[A-Z]/, ch => ch.toLowerCase()));
            if (!name) {
                const base = key || "instance";
                for (let n = 2; own(parent, key) || runtimeNames.has(key); n++) key = base + n;
            }
            if (typeof key !== "string" || !key.trim()) throw new Error("Instance names cannot be empty.");
            if (own(parent, key) || runtimeNames.has(key)) throw new Error("An entry already uses that name.");
            let data = object(definition.template) ? clone(definition.template) : {};
            put(data, "class", className);
            for (const parameter of definition.parameters || []) {
                const path = parameter.path || [];
                if (!path.length || path.includes("*") || path[0] === "class" || path[0] === "name") continue;
                if (own(parameter, "default") && get(data, path) === undefined) set(data, path, clone(parameter.default));
            }
            put(parent, key, data);
            return this.getNode(pointer(parentPath.concat(key)));
        }

        /** Removing a node leaves references visible as unresolved until edited. */
        removeNode(id) { const node = this.getNode(id); erase(this.document, node.path); }

        renameNode(id, name) {
            const node = this.getNode(id);
            if (typeof name !== "string" || !name.trim()) throw new Error("Instance names cannot be empty.");
            const parent = get(this.document, node.path.slice(0, -1));
            if (Array.isArray(parent)) throw new Error("Rename the name parameter of an instance in an array.");
            if (name !== node.key && own(parent, name)) throw new Error("An entry already uses that name.");
            if (!node.nested && this.nodes().some(other => !other.nested && other.id !== id && other.name === name)) throw new Error("Another module already uses that runtime name.");
            const incoming = this.dependencies().filter(edge => edge.targetId === id);
            // Preserve the original document's key ordering when replacing the key.
            const replacement = {};
            for (const key of Object.keys(parent)) put(replacement, key === node.key ? name : key, parent[key]);
            if (!node.nested || own(node.data, "name")) {
                if (own(node.data, "name")) put(node.data, "name", name);
            }
            if (node.path.length === 1) this.document = replacement;
            else set(this.document, node.path.slice(0, -1), replacement);
            for (const edge of incoming) {
                const insideRenamed = node.path.every((part, i) => edge.sourcePath[i] === part);
                const sourcePath = insideRenamed ? node.path.slice(0, -1).concat(name, edge.sourcePath.slice(node.path.length)) : edge.sourcePath;
                const sourceData = get(this.document, sourcePath);
                if (sourceData) set(sourceData, edge.path, name);
            }
            return this.getNode(pointer(node.path.slice(0, -1).concat(name)));
        }

        setValue(id, path, value) {
            const node = this.getNode(id);
            if (path.length === 1 && path[0] === "class" && typeof value !== "string") throw new Error("An instance class must be a string.");
            const draft = clone(node.data);
            set(draft, path, clone(value));
            set(this.document, node.path, draft);
            return this.getNode(id);
        }
        deleteValue(id, path) {
            if (path.length === 1 && path[0] === "class") throw new Error("Use Remove instance to delete a class instance.");
            erase(this.getNode(id).data, path);
        }
        replaceNode(id, input) {
            const replacement = typeof input === "string" ? JSON.parse(stripComments(input)) : clone(input);
            if (!object(replacement) || typeof replacement.class !== "string") throw new Error("An instance must be an object with a class string.");
            set(this.document, this.getNode(id).path, replacement);
            return this.getNode(id);
        }
        getValue(id, path) { return get(this.getNode(id).data, path); }

        /** Existing dependency values, with concrete paths (including array indexes). */
        dependencies(id) {
            const result = [], seen = new Set();
            for (const node of id === undefined ? this.nodes() : [this.getNode(id)]) {
                for (const definition of node.definition ? node.definition.dependencies || [] : []) {
                    const pattern = definition.path || [];
                    for (const field of expand(node.data, pattern, false)) {
                        const entries = definition.multiple && Array.isArray(field.value)
                            ? field.value.map((value, index) => ({ value, path: field.path.concat(String(index)), index }))
                            : [{ value: field.value, path: field.path }];
                        for (const entry of entries) {
                            const fullPath = node.path.concat(entry.path);
                            const identity = pointer(fullPath);
                            if (seen.has(identity)) continue;
                            seen.add(identity);
                            const target = this.resolveReference(entry.value);
                            result.push({
                                id: identity, sourceId: node.id, sourcePath: node.path, targetId: target ? target.id : null,
                                path: entry.path, fieldPath: field.path, reference: entry.value, definition,
                                index: entry.index, resolved: !!target,
                                compatible: target ? this.compatible(target.className, definition.targetClass) : false,
                                targetDisabled: !!target && target.data.bON === 0,
                            });
                        }
                    }
                }
            }
            return result;
        }

        dependencyDefinition(node, path) {
            return ((node.definition && node.definition.dependencies) || []).find(definition =>
                matches(definition.path || [], path) ||
                (definition.multiple && numeric(path[path.length - 1]) && matches(definition.path || [], path.slice(0, -1)))) || null;
        }

        /** Wildcards append rows. A multiple field appends a unique reference by default. */
        connect(sourceId, dependencyPath, targetId, options = {}) {
            const source = this.getNode(sourceId), target = this.getNode(targetId);
            const definition = this.dependencyDefinition(source, dependencyPath);
            if (!definition) throw new Error("This path is not a known dependency.");
            if (target.nested || target.className === "ModuleMgr") throw new Error("Only top-level runtime modules can be dependency targets.");
            if (target.definition && target.definition.creatable === false) throw new Error(target.className + " is not registered as a runtime module.");
            if (!target.name) throw new Error("The target needs a nonempty runtime module name.");
            if (this.nodes().some(other => !other.nested && other.id !== targetId && other.name === target.name)) throw new Error("More than one module uses that runtime name.");
            if (!this.compatible(target.className, definition.targetClass)) throw new Error("Expected " + definition.targetClass + ", but " + target.className + " is incompatible.");
            const draft = clone(source.data), path = [];
            const containers = (definition.containers || []).slice().sort((a, b) => a.path.length - b.path.length);
            const ensureContainers = () => {
                for (const container of containers) {
                    if (container.path.length > path.length) continue;
                    const concrete = path.slice(0, container.path.length);
                    if (!matches(container.path, concrete)) continue;
                    const current = get(draft, concrete);
                    if (current === undefined) set(draft, concrete, container.type === "array" ? [] : {});
                    else if (container.type === "array" ? !Array.isArray(current) : !object(current)) {
                        throw new Error(concrete.join(".") + " must contain an " + container.type + ". Edit its JSON to correct the existing value first.");
                    }
                }
            };
            for (const part of dependencyPath) {
                if (part !== "*") {
                    path.push(part);
                    ensureContainers();
                    continue;
                }
                const containerDefinition = containers.find(item => matches(item.path, path));
                if (containerDefinition && containerDefinition.type === "object") throw new Error("Choose a concrete object key for this dependency in the instance JSON.");
                const container = get(draft, path);
                if (container !== undefined && !Array.isArray(container)) throw new Error("Select a concrete key for dependencies in an object.");
                if (container === undefined) set(draft, path, []);
                path.push(String((get(draft, path) || []).length));
                ensureContainers();
            }
            const wholeField = samePath(path, definition.path || []) || matches(definition.path || [], path);
            if (definition.multiple && wholeField) {
                const current = get(draft, path);
                if (options.append === false) set(draft, path, [target.name]);
                else if (current === undefined) set(draft, path, [target.name]);
                else if (!Array.isArray(current)) throw new Error("This dependency must contain an array. Edit its JSON to correct the existing value first.");
                else if (!current.includes(target.name)) current.push(target.name);
            } else set(draft, path, target.name);
            set(this.document, source.path, draft);
            return path;
        }

        /** Pass an edge.path to remove that reference, or a field path to clear it. */
        disconnect(sourceId, path, index) {
            const source = this.getNode(sourceId);
            if (!this.dependencyDefinition(source, path)) throw new Error("This path is not a known dependency.");
            erase(source.data, index === undefined ? path : path.concat(String(index)));
        }

        validate() {
            const diagnostics = [], nodes = this.nodes();
            const report = (severity, code, message, node, path = []) => diagnostics.push({ severity, code, message, nodeId: node ? node.id : null, path });
            const names = new Map();
            if (!object(this.document.APP)) report("info", "app", "No APP settings object is present.", null);
            const includes = this.document.APP && this.document.APP.vInclude;
            if (Array.isArray(includes) && includes.length) report("info", "includes", "Included files are preserved but are not loaded by the browser. Their references may appear unresolved.", null, ["APP", "vInclude"]);
            for (const node of nodes) {
                if (!node.definition) report("warning", "unknown-class", "Class " + node.className + " is not in the catalog; its configuration is preserved.", node);
                else if (!node.nested && node.className !== "ModuleMgr" && node.definition.creatable === false) report("warning", "not-creatable", node.className + " is not registered as a top-level module.", node);
                if (!node.nested && node.className !== "ModuleMgr") {
                    if (names.has(node.name)) report("error", "duplicate-name", "Runtime module name " + node.name + " is also used by " + names.get(node.name) + ".", node);
                    else names.set(node.name, node.key);
                    if (!node.name) report("error", "empty-name", "Runtime module names cannot be empty.", node);
                }
                if (!node.definition) continue;
                for (const definition of node.definition.parameters || []) {
                    for (const field of expand(node.data, definition.path || [], !!definition.required)) {
                        if (!field.present) report("warning", "missing-parameter", "Missing parameter " + field.path.join(".") + ".", node, field.path);
                        else if (!acceptsType(field.value, definition)) report("warning", "parameter-type", field.path.join(".") + " should be " + definition.type + ".", node, field.path);
                    }
                }
                for (const definition of node.definition.dependencies || []) {
                    if (!definition.required) continue;
                    for (const field of expand(node.data, definition.path || [], true)) {
                        if (!field.present || field.value === "" || field.value === null || (Array.isArray(field.value) && !field.value.length)) {
                            report("warning", "missing-dependency", "Missing dependency " + field.path.join(".") + ".", node, field.path);
                        }
                    }
                }
            }
            for (const edge of this.dependencies()) {
                const source = this.getNode(edge.sourceId), label = edge.path.join(".");
                if (typeof edge.reference !== "string") report("warning", "dependency-type", label + " must be a module name string.", source, edge.path);
                else if (!edge.reference && !edge.definition.required) continue;
                else if (!edge.resolved) report("warning", "unresolved-dependency", label + ": module " + JSON.stringify(edge.reference) + " was not found (names match exactly).", source, edge.path);
                else if (!edge.compatible) report("error", "incompatible-dependency", label + " requires " + edge.definition.targetClass + ".", source, edge.path);
                else if (edge.targetDisabled) report("warning", "disabled-dependency", label + " refers to a disabled module.", source, edge.path);
            }
            return diagnostics;
        }
    }

    OpenKAIModel.pointer = pointer;
    OpenKAIModel.stripComments = stripComments;
    OpenKAIModel.expand = expand;
    return OpenKAIModel;
});
