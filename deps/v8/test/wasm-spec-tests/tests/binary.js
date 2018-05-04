'use strict';

let spectest = {
  print: print || ((...xs) => console.log(...xs)),
  global: 666,
  table: new WebAssembly.Table({initial: 10, maximum: 20, element: 'anyfunc'}),  memory: new WebAssembly.Memory({initial: 1, maximum: 2}),};

let registry = {spectest};

function register(name, instance) {
  registry[name] = instance.exports;
}

function module(bytes, valid = true) {
  let buffer = new ArrayBuffer(bytes.length);
  let view = new Uint8Array(buffer);
  for (let i = 0; i < bytes.length; ++i) {
    view[i] = bytes.charCodeAt(i);
  }
  let validated;
  try {
    validated = WebAssembly.validate(buffer);
  } catch (e) {
    throw new Error("Wasm validate throws");
  }
  if (validated !== valid) {
    throw new Error("Wasm validate failure" + (valid ? "" : " expected"));
  }
  return new WebAssembly.Module(buffer);
}

function instance(bytes, imports = registry) {
  return new WebAssembly.Instance(module(bytes), imports);
}

function call(instance, name, args) {
  return instance.exports[name](...args);
}

function get(instance, name) {
  return instance.exports[name];
}

function exports(name, instance) {
  return {[name]: instance.exports};
}

function run(action) {
  action();
}

function assert_malformed(bytes) {
  try { module(bytes, false) } catch (e) {
    if (e instanceof WebAssembly.CompileError) return;
  }
  throw new Error("Wasm decoding failure expected");
}

function assert_invalid(bytes) {
  try { module(bytes, false) } catch (e) {
    if (e instanceof WebAssembly.CompileError) return;
  }
  throw new Error("Wasm validation failure expected");
}

function assert_unlinkable(bytes) {
  let mod = module(bytes);
  try { new WebAssembly.Instance(mod, registry) } catch (e) {
    if (e instanceof WebAssembly.LinkError) return;
  }
  throw new Error("Wasm linking failure expected");
}

function assert_uninstantiable(bytes) {
  let mod = module(bytes);
  try { new WebAssembly.Instance(mod, registry) } catch (e) {
    if (e instanceof WebAssembly.RuntimeError) return;
  }
  throw new Error("Wasm trap expected");
}

function assert_trap(action) {
  try { action() } catch (e) {
    if (e instanceof WebAssembly.RuntimeError) return;
  }
  throw new Error("Wasm trap expected");
}

let StackOverflow;
try { (function f() { 1 + f() })() } catch (e) { StackOverflow = e.constructor }

function assert_exhaustion(action) {
  try { action() } catch (e) {
    if (e instanceof StackOverflow) return;
  }
  throw new Error("Wasm resource exhaustion expected");
}

function assert_return(action, expected) {
  let actual = action();
  if (!Object.is(actual, expected)) {
    throw new Error("Wasm return value " + expected + " expected, got " + actual);
  };
}

function assert_return_canonical_nan(action) {
  let actual = action();
  // Note that JS can't reliably distinguish different NaN values,
  // so there's no good way to test that it's a canonical NaN.
  if (!Number.isNaN(actual)) {
    throw new Error("Wasm return value NaN expected, got " + actual);
  };
}

function assert_return_arithmetic_nan(action) {
  // Note that JS can't reliably distinguish different NaN values,
  // so there's no good way to test for specific bitpatterns here.
  let actual = action();
  if (!Number.isNaN(actual)) {
    throw new Error("Wasm return value NaN expected, got " + actual);
  };
}


// binary.wast:1
let $1 = instance("\x00\x61\x73\x6d\x01\x00\x00\x00");

// binary.wast:2
let $2 = instance("\x00\x61\x73\x6d\x01\x00\x00\x00");

// binary.wast:3
let $3 = instance("\x00\x61\x73\x6d\x01\x00\x00\x00");
let $M1 = $3;

// binary.wast:4
let $4 = instance("\x00\x61\x73\x6d\x01\x00\x00\x00");
let $M2 = $4;

// binary.wast:6
assert_malformed("");

// binary.wast:7
assert_malformed("\x01");

// binary.wast:8
assert_malformed("\x00\x61\x73");

// binary.wast:9
assert_malformed("\x61\x73\x6d\x00");

// binary.wast:10
assert_malformed("\x6d\x73\x61\x00");

// binary.wast:11
assert_malformed("\x6d\x73\x61\x00\x01\x00\x00\x00");

// binary.wast:12
assert_malformed("\x6d\x73\x61\x00\x00\x00\x00\x01");

// binary.wast:13
assert_malformed("\x61\x73\x6d\x01\x00\x00\x00\x00");

// binary.wast:14
assert_malformed("\x77\x61\x73\x6d\x01\x00\x00\x00");

// binary.wast:15
assert_malformed("\x7f\x61\x73\x6d\x01\x00\x00\x00");

// binary.wast:16
assert_malformed("\x80\x61\x73\x6d\x01\x00\x00\x00");

// binary.wast:17
assert_malformed("\x82\x61\x73\x6d\x01\x00\x00\x00");

// binary.wast:18
assert_malformed("\xff\x61\x73\x6d\x01\x00\x00\x00");

// binary.wast:21
assert_malformed("\x00\x00\x00\x01\x6d\x73\x61\x00");

// binary.wast:24
assert_malformed("\x61\x00\x6d\x73\x00\x01\x00\x00");

// binary.wast:25
assert_malformed("\x73\x6d\x00\x61\x00\x00\x01\x00");

// binary.wast:28
assert_malformed("\x00\x41\x53\x4d\x01\x00\x00\x00");

// binary.wast:31
assert_malformed("\x00\x81\xa2\x94\x01\x00\x00\x00");

// binary.wast:34
assert_malformed("\xef\xbb\xbf\x00\x61\x73\x6d\x01\x00\x00\x00");

// binary.wast:36
assert_malformed("\x00\x61\x73\x6d");

// binary.wast:37
assert_malformed("\x00\x61\x73\x6d\x01");

// binary.wast:38
assert_malformed("\x00\x61\x73\x6d\x01\x00\x00");

// binary.wast:39
assert_malformed("\x00\x61\x73\x6d\x00\x00\x00\x00");

// binary.wast:40
assert_malformed("\x00\x61\x73\x6d\x0d\x00\x00\x00");

// binary.wast:41
assert_malformed("\x00\x61\x73\x6d\x0e\x00\x00\x00");

// binary.wast:42
assert_malformed("\x00\x61\x73\x6d\x00\x01\x00\x00");

// binary.wast:43
assert_malformed("\x00\x61\x73\x6d\x00\x00\x01\x00");

// binary.wast:44
assert_malformed("\x00\x61\x73\x6d\x00\x00\x00\x01");
