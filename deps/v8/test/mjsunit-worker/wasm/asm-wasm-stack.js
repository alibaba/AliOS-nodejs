// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --validate-asm --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/wasm/asm-wasm-stack.js");
	}
}
var filename = '(?:[^ ]+/)?test/mjsunit-worker/wasm/asm-wasm-stack.js';
filename = filename.replace(/\//g, '[/\\\\]');

function checkPreformattedStack(e, expected_lines) {
  print('preformatted stack: ' + e.stack);
  var lines = e.stack.split('\n');
  assertEquals(expected_lines.length, lines.length);
  for (var i = 0; i < lines.length; ++i) {
    assertMatches(expected_lines[i], lines[i], 'line ' + i);
  }
}

function printCallsites(stack) {
  print('callsite objects (size ' + stack.length + '):');
  for (var i = 0; i < stack.length; ++i) {
    var s = stack[i];
    print(
        ' [' + i + '] ' + s.getFunctionName() + ' (' + s.getFileName() + ':' +
        s.getLineNumber() + ':' + s.getColumnNumber() + ')');
  }
}

function checkCallsiteArray(stack, expected) {
  assertEquals(expected.length, stack.length, 'stack size');
  for (var i = 0; i < expected.length; ++i) {
    var cs = stack[i];
    assertMatches('^' + filename + '$', cs.getFileName(), 'file name at ' + i);
    assertEquals(expected[i][0], cs.getFunctionName(), 'function name at ' + i);
    assertEquals(expected[i][1], cs.getLineNumber(), 'line number at ' + i);
    assertEquals(expected[i][2], cs.getColumnNumber(), 'column number at ' + i);
    assertNotNull(cs.getThis(), 'receiver should be global');
    assertEquals(stack[0].getThis(), cs.getThis(), 'receiver should be global');
  }
}

function checkFunctionsOnCallsites(e, expected) {
  printCallsites(e.stack);
  checkCallsiteArray(e.stack, expected);
}

function checkTopFunctionsOnCallsites(e, expected) {
  printCallsites(e.stack);
  assertTrue(
      e.stack.length >= expected.length, 'expected at least ' +
          expected.length + ' callsites, got ' + e.stack.length);
  checkCallsiteArray(e.stack.slice(0, expected.length), expected);
}

function throwException() {
  throw new Error('exception from JS');
}

function generateWasmFromAsmJs(stdlib, foreign) {
  'use asm';
  var throwFunc = foreign.throwFunc;
  function callThrow() {
    throwFunc();
  }
  function redirectFun(i) {
    i = i | 0;
    switch (i | 0) {
      case 0: callThrow(); break;
      case 1: redirectFun(0); break;
      case 2: redirectFun(1); break;
      case 3: funTable[i & 0](2); break;
      case 4: forwardFun(); break;
    }
  }
  function forwardFun() {
    redirectFun(3);
  }
  var funTable = [ redirectFun ];
  return redirectFun;
}

(function PreformattedStackTraceFromJS() {
  var fun = generateWasmFromAsmJs(this, {throwFunc: throwException});
  assertTrue(%IsWasmCode(fun));
  var e = null;
  try {
    fun(0);
  } catch (ex) {
    e = ex;
  }
  assertInstanceof(e, Error, 'exception should have been thrown');
  checkPreformattedStack(e, [
    '^Error: exception from JS$',
    '^ *at throwException \\(' + filename + ':61:9\\)$',
    '^ *at callThrow \\(' + filename + ':68:5\\)$',
    '^ *at redirectFun \\(' + filename + ':73:15\\)$',
    '^ *at PreformattedStackTraceFromJS \\(' + filename + ':92:5\\)$',
    '^ *at ' + filename + ':105:3$'
  ]);
})();

// Now collect the Callsite objects instead of just a string.
Error.prepareStackTrace = function(error, frames) {
  return frames;
};

(function CallsiteObjectsFromJS() {
  var fun = generateWasmFromAsmJs(this, {throwFunc: throwException});
  assertTrue(%IsWasmCode(fun));
  var e = null;
  try {
    fun(4);
  } catch (ex) {
    e = ex;
  }
  assertInstanceof(e, Error, 'exception should have been thrown');
  checkFunctionsOnCallsites(e, [
    ['throwException', 61, 9],          // --
    ['callThrow', 68, 5],               // --
    ['redirectFun', 73, 15],            // --
    ['redirectFun', 74, 15],            // --
    ['redirectFun', 75, 15],            // --
    ['redirectFun', 76, 30],            // --
    ['forwardFun', 81, 5],             // --
    ['redirectFun', 77, 15],            // --
    ['CallsiteObjectsFromJS', 117, 5],  // --
    [null, 134, 3]
  ]);
})();

function generateOverflowWasmFromAsmJs() {
  'use asm';
  function f(a) {
    a = a | 0;
    return f(a) | 0;
  }
  return f;
}

(function StackOverflowPosition() {
  var fun = generateOverflowWasmFromAsmJs();
  assertTrue(%IsWasmCode(fun));
  var e = null;
  try {
    fun(23);
  } catch (ex) {
    e = ex;
  }
  assertInstanceof(e, RangeError, 'RangeError should have been thrown');
  checkTopFunctionsOnCallsites(e, [
    ['f', 138, 13],  // --
    ['f', 140, 12],  // --
    ['f', 140, 12],  // --
    ['f', 140, 12]   // --
  ]);
})();
