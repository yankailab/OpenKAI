#!/usr/bin/env python3
"""Generate the offline editor catalog from source. Python 3, standard library only."""
import argparse
import ast
import copy
import hashlib
import json
import operator
import re
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
OUT = ROOT / 'html/console/editor'
SUFFIXES = {'.h', '.hpp', '.hh', '.cpp', '.cc', '.cxx'}
MISSING = object()
TOKEN = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*[\s\S]*?\*/')


def uncomment(text):
    return TOKEN.sub(lambda m: re.sub(r'[^\n]', ' ', m[0]) if m[0].startswith(('/', '/*')) else m[0], text)


def closing(text, pos, left='{', right='}'):
    depth, quote, escape = 0, None, False
    for i in range(pos, len(text)):
        c = text[i]
        if quote:
            if escape:
                escape = False
            elif c == '\\':
                escape = True
            elif c == quote:
                quote = None
        elif c in ('"', "'"):
            quote = c
        elif c == left:
            depth += 1
        elif c == right:
            depth -= 1
            if depth == 0:
                return i
    raise ValueError('Unbalanced source delimiter')


def split_args(text):
    result, start, stack, quote, escape = [], 0, [], None, False
    for i, c in enumerate(text):
        if quote:
            if escape:
                escape = False
            elif c == '\\':
                escape = True
            elif c == quote:
                quote = None
        elif c in ('"', "'"):
            quote = c
        elif c in '([{':
            stack.append(c)
        elif c in ')]}':
            if stack:
                stack.pop()
        elif c == ',' and not stack:
            result.append(text[start:i].strip())
            start = i + 1
    result.append(text[start:].strip())
    return result


def source_ref(path, text, offset):
    return f'{path}:{text.count(chr(10), 0, offset) + 1}'


def literal(expr, constants, seen=()):
    expr = expr.strip()
    if expr in ('true', 'false'):
        return expr == 'true'
    if expr.startswith('"') and expr.endswith('"'):
        try:
            return json.loads(expr)
        except ValueError:
            return MISSING
    if expr in constants and expr not in seen:
        return literal(constants[expr], constants, seen + (expr,))
    vector = re.fullmatch(r'(?:Eigen::)?(?:Vector\w+|vec\w+)\s*[({](.*)[)}]', expr, re.S)
    if vector or (expr.startswith('{') and expr.endswith('}')):
        parts = split_args(vector[1] if vector else expr[1:-1])
        values = [literal(p, constants, seen) for p in parts if p]
        return values if all(v is not MISSING for v in values) else MISSING
    # Numeric C++ suffixes, simple arithmetic and named constants only; no eval().
    normalized = re.sub(r'(?<=\d)[uUlLfF]+\b', '', expr)
    try:
        tree = ast.parse(normalized, mode='eval')
        ops = {ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
               ast.Div: operator.truediv, ast.FloorDiv: operator.floordiv,
               ast.LShift: operator.lshift, ast.RShift: operator.rshift,
               ast.BitOr: operator.or_, ast.BitAnd: operator.and_}
        def visit(n):
            if isinstance(n, ast.Constant) and isinstance(n.value, (int, float)):
                return n.value
            if isinstance(n, ast.Name) and n.id in constants and n.id not in seen:
                return literal(constants[n.id], constants, seen + (n.id,))
            if isinstance(n, ast.UnaryOp) and isinstance(n.op, (ast.USub, ast.UAdd)):
                v = visit(n.operand)
                return -v if isinstance(n.op, ast.USub) else v
            if isinstance(n, ast.BinOp) and type(n.op) in ops:
                return ops[type(n.op)](visit(n.left), visit(n.right))
            raise ValueError()
        value = visit(tree.body)
        if isinstance(value, (int, float)) and abs(value) <= 9007199254740991:
            return value
    except (ValueError, SyntaxError, TypeError, ZeroDivisionError, OverflowError):
        pass
    return MISSING


def infer_type(cpp_type, variable, key, template=''):
    cpp_type = cpp_type.replace('std::', '').strip()
    if re.search(r'vector\s*<|array\s*<|Vector\d|vec\d|\[', cpp_type):
        return 'array', 'declaration'
    if 'map<' in cpp_type or 'map <' in cpp_type:
        return 'json', 'declaration'
    if 'optional<' in cpp_type:
        return infer_type(cpp_type[cpp_type.index('<') + 1:-1], variable, key, template)
    if re.search(r'\bbool\b', cpp_type):
        return 'boolean', 'declaration'
    if re.search(r'\bstring\b|char\s*\*', cpp_type):
        return 'string', 'declaration'
    if re.search(r'\b(?:int|uint\d+_t|int\d+_t|unsigned|long|short|size_t|uchar|BYTE)\b', cpp_type):
        return 'integer', 'declaration'
    if re.search(r'\b(?:float|double)\b', cpp_type):
        return 'number', 'declaration'
    if cpp_type == 'json':
        return 'json', 'declaration'
    if template and not re.search(r'\.(?:x|y|z|w)\(\)|\[', variable):
        return 'array', 'jKv-vector-overload'
    if key.startswith('_'):
        return 'string', 'key-convention'
    if re.match(r'b[A-Z]', key):
        return 'boolean', 'key-convention'
    if re.match(r'v[A-Z]', key):
        return 'array', 'key-convention'
    return 'json', 'unresolved'


def declarations(text):
    result = {}
    pattern = r'(?:^|(?<=[;{}]))\s*(?:static\s+|const\s+|constexpr\s+|mutable\s+)*((?:unsigned\s+|signed\s+)?[\w:]+(?:\s*<[^;{}]+?>)?\s*\*?)\s*(\w+(?:\s*\[[^\]]*\])?\s*(?:(?:=|\{)[^;]+)?(?:,[^;]+)?);'
    for m in re.finditer(pattern, text, re.M):
        if m[1].strip() in ('return', 'delete', 'throw', 'using', 'typedef', 'break', 'continue'):
            continue
        for item in split_args(m[2]):
            declaration = re.fullmatch(r'(\w+)\s*(\[[^\]]*\])?\s*(?:=\s*(.*)|(\{.*\}))?', item, re.S)
            if declaration:
                result[declaration[1]] = {'cppType': m[1].strip() + (declaration[2] or ''), 'expr': declaration[3] or declaration[4]}
    return result


def extract_methods(text, path):
    methods = []
    pattern = r'\b(\w+)::(~?\w+)\s*\([^;{}]*\)\s*(?:const\s*)?(?::[^{};]+)?\s*\{'
    for m in re.finditer(pattern, text):
        p = m.end() - 1
        end = closing(text, p)
        methods.append({'class': m[1], 'name': m[2], 'body': text[p+1:end], 'offset': p+1,
                        'source': source_ref(path, text, m.start()), 'file': path, 'full': text})
    return methods


def registration(text):
    stack, result = [], {}
    for line in text.splitlines():
        directive = re.match(r'\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b\s*(.*)', line)
        if directive:
            op, condition = directive.groups()
            if op in ('if', 'ifdef', 'ifndef'):
                stack.append((op + ' ' + condition).strip())
            elif op in ('else', 'elif') and stack:
                stack[-1] = op + (' ' + condition if condition else '')
            elif op == 'endif' and stack:
                stack.pop()
        for m in re.finditer(r'\bADD_MODULE\(\s*(\w+)\s*\)', line):
            result.setdefault(m[1], []).append(list(stack))
    return result


def generate():
    paths = sorted(p for p in (ROOT / 'src').rglob('*') if p.suffix in SUFFIXES and 'Dependencies' not in p.relative_to(ROOT).parts)
    files = {str(p.relative_to(ROOT)): uncomment(p.read_text(errors='replace')) for p in paths}
    constants = {}
    for text in files.values():
        constants.update(dict(re.findall(r'^[ \t]*#define[ \t]+(\w+)[ \t]+([^\n\\]+)', text, re.M)))
        constants.update(dict(re.findall(r'\b(?:const|constexpr)\s+(?:int|float|double|uint\d+_t)\s+(\w+)\s*=\s*([^;]+);', text)))
    classes, records, all_methods = {}, {}, []
    variants = {}
    for path, text in files.items():
        for m in re.finditer(r'(?<!enum )\b(class|struct)\s+(\w+)\s*(?:final\s*)?(?::([^;{}]+))?\s*\{', text):
            if re.search(r'\benum\s*$', text[max(0,m.start()-10):m.start()]):
                continue
            end = closing(text, m.end()-1)
            body = text[m.end():end]
            bases = [re.sub(r'\b(?:public|protected|private|virtual)\s+', '', b).strip().split('<')[0] for b in (m[3] or '').split(',') if b.strip()]
            record = {'name':m[2], 'category':str(Path(path).parent.relative_to('src')), 'source':source_ref(path,text,m.start()),
                      'baseClasses':bases, 'parameters':[], 'dependencies':[], 'containers':[], 'diagnostics':[],
                      '_members':declarations(body), '_methods':[], '_embedded':[], '_defaultOverrides':{}}
            if m[1] == 'class':
                variants.setdefault(m[2],[]).append({'source':record['source'],'category':record['category']})
                classes[m[2]] = record
                records[m[2]] = record
            elif m[2] not in classes:
                records[m[2]] = record
        all_methods.extend(extract_methods(text, path))
    for method in all_methods:
        if method['class'] in records:
            records[method['class']]['_methods'].append(method)
    registered = registration(files['src/Module/Module.cpp'])
    missing_registered = sorted(set(registered) - set(classes))
    # Scalar and vector constructors can establish defaults after in-class initializers.
    for name, record in records.items():
        for method in record['_methods']:
            if method['name'] != name:
                continue
            for m in re.finditer(r'\b(m_\w+)\s*=\s*([^;]+);', method['body']):
                record['_defaultOverrides'][m[1]]=m[2]
                if m[1] in record['_members']:
                    record['_members'][m[1]]['expr'] = m[2]

    def symbol(record, variable, local):
        cast = re.match(r'\(\s*(int|float|double|bool)\s*&?\s*\)',variable)
        if cast:
            return {'cppType':cast[1]}
        parts = re.split(r'\.|->', variable.strip())
        first = re.sub(r'\[.*|\(.*', '', parts[0])
        entry = local.get(first)
        def member(r, key, seen=()):
            if key in r['_members']:
                return r['_members'][key]
            for base in r['baseClasses']:
                if base in records and base not in seen:
                    found = member(records[base], key, seen+(base,))
                    if found:
                        return found
        entry = entry or member(record, first) or {}
        for part in parts[1:]:
            if re.match(r'[xyzw]\(', part):
                return {'cppType':'int' if entry.get('cppType','').endswith('i') else 'float'}
            nested = records.get(entry.get('cppType','').replace('const ', '').replace('*','').strip())
            entry = member(nested, part) if nested else {}
            entry = entry or {}
        return entry

    audit = Counter()
    for name, record in records.items():
        for method in record['_methods']:
            if method['name'] not in ('loadConfig', 'link') and not (name == 'ROS_fastLio' and method['name'] == 'init'):
                continue
            body = method['body']
            local = declarations(body)
            aliases = {'m_pJ': []}
            if name == 'ROS_fastLio' and method['name'] == 'init':
                aliases['j'] = []
            values, iterators, vector_items = {}, {}, {}
            events = []
            # Statements create aliases in source order, including embedded JSON objects.
            for m in re.finditer(r'\b(?:const\s+)?(?:json|auto)\s*[*&]?\s*(\w+)\s*=\s*([^;]+);', body):
                events.append((m.start(), 'alias', m))
            for m in re.finditer(r'\b(?:auto\s+)?(\w+)\s*=\s*(\w+)\.begin\s*\(', body):
                events.append((m.start(), 'iterator', m))
            for m in re.finditer(r'for\s*\(\s*(?:const\s+)?(?:[\w:]+(?:<[^>]+>)?)\s*[*&]?\s*(\w+)\s*:\s*(\w+)\s*\)', body):
                events.append((m.start(), 'range', m))
            for m in re.finditer(r'\bjKv(?:<([^>]+)>)?\s*\(', body):
                events.append((m.start(), 'read', m))
            for m in re.finditer(r'\bfindModule\s*\(', body):
                events.append((m.start(), 'dependency', m))
            def resolve(expr):
                expr = expr.strip()
                while expr.startswith('(') and closing(expr,0,'(',')') == len(expr)-1:
                    expr = expr[1:-1].strip()
                expr = expr.lstrip('*&')
                if expr in aliases:
                    return aliases[expr]
                access = re.fullmatch(r'(\w+)(?:\[([^]]+)\]|\.at\(([^)]+)\))', expr)
                if access and access[1] in aliases:
                    part = access[2] or access[3]
                    return aliases[access[1]] + ([json.loads(part)] if part.startswith('"') else ['*'])
                access = re.fullmatch(r'\(\*(\w+)\)\[([^]]+)\]', expr)
                if access and access[1] in aliases:
                    return aliases[access[1]] + ['*']
                value = re.fullmatch(r'(\w+)\.value\(\)', expr)
                if value and value[1] in iterators:
                    return iterators[value[1]] + ['*']
                return None
            def ref(pos):
                return source_ref(method['file'], method['full'], method['offset']+pos)
            for pos, kind, m in sorted(events, key=lambda e:e[0]):
                if kind == 'iterator':
                    if m[2] in aliases:
                        iterators[m[1]] = aliases[m[2]]
                elif kind == 'range':
                    if m[2] in aliases:
                        aliases[m[1]] = aliases[m[2]] + ['*']
                    elif m[2] in values:
                        vector_items[m[1]] = values[m[2]]
                elif kind == 'alias':
                    var, expr = m[1], m[2].strip()
                    key = re.match(r'jK\s*\(\s*([^,]+),\s*"([^"]+)"\s*\)', expr)
                    if key:
                        parent = resolve(key[1])
                        if parent is not None:
                            path = parent+[key[2]]
                            aliases[var] = path
                            checks = re.findall(r'\b'+re.escape(var)+r'\s*(?:->|\.)\s*is_(object|array)\s*\(', body)
                            type_ = checks[0] if checks else 'json'
                            record['parameters'].append({'path':path,'type':type_,'declaredIn':name,'source':ref(pos)})
                            if checks:
                                record['containers'].append({'path':path,'type':type_})
                    else:
                        path = resolve(expr)
                        if path is not None:
                            aliases[var] = path
                elif kind == 'read':
                    audit['configReads'] += 1
                    start = m.end()-1
                    args = split_args(body[start+1:closing(body,start,'(',')')])
                    if len(args) < 3 or not re.fullmatch(r'"[^"]+"', args[1]):
                        record['diagnostics'].append({'source':ref(pos),'kind':'dynamic-key','expression':body[m.start():closing(body,start,'(',')')+1]})
                        audit['unresolvedReads'] += 1
                        continue
                    parent = resolve(args[0])
                    if parent is None:
                        record['diagnostics'].append({'source':ref(pos),'kind':'unresolved-json-alias','expression':args[0],'key':json.loads(args[1])})
                        audit['unresolvedReads'] += 1
                        continue
                    key, variable = json.loads(args[1]), args[2]
                    path = parent+[key]
                    values[variable] = path
                    sym = copy.copy(symbol(record, variable, local))
                    preceding=list(re.finditer(r'(?<![\w.])'+re.escape(variable)+r'\s*=\s*([^;]+);',body[:pos]))
                    if preceding:
                        sym['expr']=preceding[-1][1]
                    type_, evidence = infer_type(sym.get('cppType',''), variable, key, m[1] or '')
                    parameter = {'path':path,'type':type_,'declaredIn':name,'source':ref(pos),'typeEvidence':evidence,'cppVariable':variable}
                    if sym.get('cppType'):
                        parameter['cppType'] = sym['cppType']
                    if sym.get('expr'):
                        default = literal(sym['expr'], constants)
                        if default is not MISSING and type_ != 'json':
                            parameter['default'] = bool(default) if type_=='boolean' else int(default) if type_=='integer' and isinstance(default,(int,float)) else default
                        else:
                            parameter['defaultExpression'] = sym['expr'].strip()
                    record['parameters'].append(parameter)
                    audit['resolvedReads'] += 1
                elif kind == 'dependency':
                    audit['moduleLookups'] += 1
                    start = m.end()-1
                    arg = body[start+1:closing(body,start,'(',')')].strip()
                    path = vector_items.get(arg) or values.get(arg)
                    if not path:
                        record['diagnostics'].append({'source':ref(pos),'kind':'unresolved-module-reference','expression':arg})
                        audit['unresolvedLookups'] += 1
                        continue
                    statement_start = body.rfind(';',0,pos)+1
                    statement_end = body.find(';',pos)
                    statement = body[statement_start:statement_end]
                    casts = re.findall(r'(?:dynamic_cast|static_cast)\s*<\s*(\w+)\s*\*', statement)
                    casts += re.findall(r'\(\s*(\w+)\s*\*\s*\)', statement)
                    target = next((c for c in casts if c != 'BASE'), casts[0] if casts else 'BASE')
                    if target == 'BASE' and path[-1] in records:
                        target = path[-1]
                    elif target == 'BASE' and path[-1].startswith('v') and '_'+path[-1][1:] in records:
                        target = '_'+path[-1][1:]
                    # A subsequent dynamic_cast refines the generic BASE lookup.
                    if target == 'BASE':
                        next_cast = re.search(r'dynamic_cast\s*<\s*(\w+)\s*\*',body[statement_end:statement_end+1400])
                        if next_cast:
                            target = next_cast[1]
                    dep = {'path':path,'targetClass':target,'multiple':arg in vector_items,'declaredIn':name,'source':ref(pos)}
                    tail=body[statement_end+1:body.find(';',statement_end+1)+1]
                    if not dep['multiple'] and '*' not in path and re.match(r'\s*(?:NULL_F\(|IF_Le_F\(!)',tail):
                        dep['required']=True
                    record['dependencies'].append(dep)
                    audit['resolvedLookups'] += 1
            for m in re.finditer(r'createThread\s*\(\s*jK\s*\([^,]+,\s*"([^"]+)"\s*\)',body):
                record['_embedded'].append(([m[1]], '_Thread'))
                record['containers'].append({'path':[m[1]],'type':'object'})

    # Audited adapters for composed/dynamic readers that are not simple jKv aliases.
    adapters(records, files, constants, symbol)
    flattened = {}
    def flatten(name, trail=()):
        if name in flattened:
            return flattened[name]
        if name in trail:
            raise ValueError('Inheritance cycle: '+name)
        own = records[name]
        result = {k:copy.deepcopy(v) for k,v in own.items() if not k.startswith('_')}
        for field in ('parameters','dependencies','containers'):
            entries = {}
            for base in own['baseClasses']:
                if base in records:
                    for item in flatten(base,trail+(name,))[field]:
                        entries[tuple(item['path'])] = copy.deepcopy(item)
            for item in own[field]:
                entries[tuple(item['path'])] = copy.deepcopy(item)
            for prefix, embedded in own['_embedded']:
                for item in flatten(embedded,trail+(name,))[field]:
                    if item['path'] in (['name'],['class'],['bON']):
                        continue
                    item = copy.deepcopy(item)
                    item['path'] = prefix+item['path']
                    item['embeddedClass'] = embedded
                    entries[tuple(item['path'])] = item
            result[field] = sorted(entries.values(),key=lambda i:tuple(i['path']))
        for parameter in result['parameters']:
            variable=parameter.get('cppVariable')
            if variable in own['_defaultOverrides']:
                expression=own['_defaultOverrides'][variable]
                parameter.pop('default',None)
                value=literal(expression,constants)
                if value is not MISSING:
                    parameter['default']=bool(value) if parameter['type']=='boolean' else value
                    parameter.pop('defaultExpression',None)
                else:
                    parameter['defaultExpression']=expression
        if len(variants.get(name,[]))>1:
            result['declarations']=variants[name]
        result['creatable'] = name in registered
        result['buildConditions'] = registered.get(name,[])
        result['configurable'] = bool(result['parameters'] or result['dependencies'])
        flattened[name] = result
        return result
    catalog = [flatten(name) for name in sorted(classes,key=lambda n:(classes[n]['category'],n.lower()))]
    for entry in catalog:
        dep_paths = {tuple(d['path']) for d in entry['dependencies']}
        for parameter in entry['parameters']:
            if tuple(parameter['path']) in dep_paths:
                parameter['dependency'] = True
                parameter.pop('default',None)
        for dep in entry['dependencies']:
            dep['containers'] = [c for c in entry['containers'] if dep['path'][:len(c['path'])] == c['path']]
    digest = hashlib.sha256()
    for path in paths:
        digest.update(str(path.relative_to(ROOT)).encode()+b'\0'+path.read_bytes()+b'\0')
    return {'schemaVersion':1,'title':'OpenKAI editor class catalog',
            'description':'Source-derived editor metadata; not a JSON Schema validator. See SCHEMA.md.',
            'generator':'tools/generate-schema.py','sourceDigest':digest.hexdigest(),
            'categories':sorted({c['category'] for c in catalog}), 'classes':catalog,
            'application':{'key':'APP','class':'ModuleMgr','parameters':[
                {'path':['class'],'type':'string','default':'ModuleMgr'},
                {'path':['bStdErr'],'type':'boolean','default':True,'source':'src/Module/ModuleMgr.cpp:bStdErr'},
                {'path':['vInclude'],'type':'array','description':'Additional JSON paths resolved by OpenKAI at launch; the browser does not load them automatically.','source':'src/Module/ModuleMgr.cpp:loadJsonFiles'}]},
            'audit':{'sourceFiles':len(paths),'classCount':len(catalog),'classDeclarations':sum(len(v) for v in variants.values()),'creatableClassCount':sum(c['creatable'] for c in catalog),'factoryClassesWithoutDeclaration':missing_registered,
                     'sourceFilesScanned':[str(p.relative_to(ROOT)) for p in paths],**dict(audit),
                     'diagnosticsCount':sum(len(c['diagnostics']) for c in records.values()),
                     'diagnostics':[dict(d,className=c['name']) for c in records.values() for d in c['diagnostics']],
                     'limitations':['Static extraction does not evaluate C++ preprocessing or validate a particular binary build.',
                                    'Methods other than loadConfig/link are not treated as configuration; runtime console commands are excluded.',
                                    'Only safely parsed literal/arithmetic defaults are emitted. Missing default means unknown, not required.',
                                    'Dynamic/composed readers use audited adapters; diagnostics identify unresolved readers. Keep unknown JSON fields when round-tripping.',
                                    'A wildcard path can denote an array item or object member; containers records the parent shape.']}}


def adapters(records, files, constants, symbol):
    def param(name,path,type_,source,**extra):
        records[name]['parameters'].append({'path':path,'type':type_,'source':source,'declaredIn':name,'extraction':'audited-adapter',**extra})
    def dep(name,path,target,source,multiple=False):
        records[name]['dependencies'].append({'path':path,'targetClass':target,'multiple':multiple,'source':source,'declaredIn':name,'extraction':'audited-adapter'})
    param('ModuleMgr',['bStdErr'],'boolean','src/Module/ModuleMgr.cpp:bStdErr',default=True)
    param('ModuleMgr',['vInclude'],'array','src/Module/ModuleMgr.cpp:loadJsonFiles',description='Additional config paths loaded by the OpenKAI runtime.')
    # ModuleMgr's switch is not read by the individual class.
    param('BASE',['bON'],'integer','src/Module/ModuleMgr.cpp:createAll',default=1,description='0 disables this instance; nonzero enables it.')
    # StateBase objects live under a keyed map, not in the module factory.
    records['_StateControl']['_embedded'].append((['states','*'],'StateBase'))
    records['_StateControl']['containers'].append({'path':['states'],'type':'object'})
    # A shared helper chooses keys at runtime and is called by both viewer classes.
    helper='src/UI/Viewer/SelectableOctGridSources.cpp:link'
    for name in ('_WebSelectableOctGrid','_ImGUIselectableOctGrid'):
        for key,target in (('vGeometry','_GeometryBase'),('vSelectableOctGrid','_SelectableOctGrid')):
            records[name]['containers'].append({'path':[key],'type':'array'})
            param(name,[key],'array',helper)
            dep(name,[key,'*',target],target,helper)
            param(name,[key,'*',target],'string',helper,dependency=True)
            for field,type_,default in [('bVisible','boolean',True),('matCol','array',[1,1,1,1]),('matLineWidth','number',1)]:
                param(name,[key,'*',field],type_,helper,default=default)
            fields=['nC'] if key == 'vSelectableOctGrid' else ['nP','nL','matPointSize']
            for field in fields:
                param(name,[key,'*',field],'number' if field=='matPointSize' else 'integer',helper,**({'default':2} if field=='matPointSize' else {}))
    # Preserve the more specific runtime peer cast used by the routing consumer.
    for dependency in records['_Mavlink']['dependencies']:
        if dependency['path'] == ['vRoutings']:
            dependency['targetClass'] = '_Mavlink'
            dependency['extraction'] = 'audited-adapter'
    dep('_APmav_drive',['_SelectableOctGrid'],'_SelectableOctGrid','src/Autopilot/FC/ArduPilot/_APmav_drive.cpp:link')
    for dependency in records['_APmav_drive']['dependencies']:
        if dependency['path'] in (['_SelectableOctGrid'],['_OctreeGrid']):
            dependency.pop('required',None)
            dependency['description']='_SelectableOctGrid is preferred; _OctreeGrid is a legacy fallback.'
    records['_ROS_fastLio']['_embedded'].append((['node'],'ROS_fastLio'))
    records['_ROS_fastLio']['containers'].append({'path':['node'],'type':'object'})
    param('_ROS_fastLio',['node'],'object','src/ROS/_ROS_fastLio.cpp:loadConfig')
    param('_Canvas',['vRoi'],'array','src/Canvas/_Canvas.cpp:loadConfig',default=[0,0,1,1])
    # Scepter startup consumes every key exposed by visitScControls, not jKv calls.
    sc_path='src/Vision/RGBD/_Scepter.cpp'
    for match in re.finditer(r'\bf\("([^\"]+)",\s*([^,]+),\s*"([^\"]+)"\)', files[sc_path]):
        key,variable,group=match.groups()
        expression=variable.replace('c.', 'm_scCtrl.')
        sym=symbol(records['_Scepter'],expression,{})
        type_,_=infer_type(sym.get('cppType',''),expression,key)
        if any(x in key for x in ('ExposureTimes','AIModuleParams','InputFrames','PreviewFrames')):
            type_='array'
        elif type_=='json':
            if variable=='ntpIP': type_='string'
            elif variable.endswith('.enable'): type_='boolean'
            elif any(x in key for x in ('ExposureTimes','AIModuleParams','InputFrames','PreviewFrames')): type_='array'
            else: type_='integer'
        # Do not materialize SDK hardware overrides merely by adding a camera.
        param('_Scepter',[key],type_,source_ref(sc_path,files[sc_path],match.start()),group=group)
    param('_Scepter',['thread'],'object',sc_path+':loadConfig')
    param('_Scepter',['pclStride'],'integer',sc_path+':controlSchema',default=1)
    # Orbbec contains an explicit catalog for optional hardware overrides.
    ob_path='src/Vision/RGBD/_Orbbec.cpp'
    ob_text=files[ob_path]
    type_map={'bool':'boolean','int':'integer','float':'number'}
    for match in re.finditer(r'\{\{"key",\s*"([^\"]+)"\},\s*\{"type",\s*"([^\"]+)"\},\s*\{"category",\s*"([^\"]+)"\}', ob_text):
        key,type_,group=match.groups()
        param('_Orbbec',[key],type_map.get(type_,type_),source_ref(ob_path,ob_text,match.start()),group=group,nullable=True)
    for method in records['_Orbbec']['_methods']:
        if method['name']!='configValues': continue
        for match in re.finditer(r'\{"([^\"]+)",\s*(m_\w+)\}',method['body']):
            key,variable=match.groups()
            sym=symbol(records['_Orbbec'],variable,{})
            type_,_=infer_type(sym.get('cppType',''),variable,key)
            param('_Orbbec',[key],type_,ob_path+':configValues')
    # Document nonlocal reads and dynamic helper reads without attributing them to callers.
    for name,record in records.items():
        for diagnostic in record['diagnostics']:
            if name=='SelectableOctGridSources':
                diagnostic['resolution']='Covered by the viewer adapters in this generator.'
            elif name=='_WebGeometryBase' and diagnostic.get('expression')=='*pConfig':
                diagnostic['resolution']='Reads bON from the referenced module, not this instance.'
    # The field below is read by UUID-specific helpers rather than jKv.
    param('_SelectableOctGrid',['vSelectedCells'],'array','src/Universe/Grid/_SelectableOctGrid.cpp:loadConfig',description='Selected cell UUID strings; each ID must refer to the configured grid.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check',action='store_true',help='Fail if checked-in catalog/mirror differs from regenerated output.')
    parser.add_argument('--overrides',type=Path,help='Optional JSON overrides: {classes:{Class:{parameters:[],dependencies:[],...}}}; path entries replace generated entries.')
    args = parser.parse_args()
    schema = generate()
    if args.overrides:
        overrides=json.loads(args.overrides.read_text())
        by_name={c['name']:c for c in schema['classes']}
        for name,change in overrides.get('classes',{}).items():
            if name not in by_name:
                raise ValueError('Unknown class in overrides: '+name)
            for field,value in change.items():
                if field in ('parameters','dependencies','containers'):
                    combined={tuple(item['path']):item for item in by_name[name][field]}
                    for item in value:
                        combined[tuple(item['path'])]=item
                    by_name[name][field]=list(combined.values())
                else:
                    by_name[name][field]=value
        schema['overrides']=str(args.overrides)
    pretty=json.dumps(schema,indent=2,ensure_ascii=False)+'\n'
    compact=json.dumps(schema,separators=(',',':'),ensure_ascii=False)
    mirror='// Generated by tools/generate-schema.py; edit the source or use --overrides.\nwindow.OPENKAI_SCHEMA = '+compact+';\n'
    outputs={OUT/'OpenKAI.json':pretty,OUT/'js/schema-data.js':mirror}
    if args.check:
        stale=[str(p.relative_to(ROOT)) for p,text in outputs.items() if not p.exists() or p.read_text()!=text]
        if stale:
            raise SystemExit('Schema is stale: '+', '.join(stale))
    else:
        for path,text in outputs.items():
            path.parent.mkdir(parents=True,exist_ok=True)
            path.write_text(text)
    print(json.dumps({k:v for k,v in schema['audit'].items() if k not in ('sourceFilesScanned','limitations','diagnostics')},indent=2))


if __name__=='__main__':
    main()
