// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/osr-forin-nested.js");
	}
}
function test(e, f, v) {
  assertEquals(e, f(v));
  assertEquals(e, f(v));
  assertEquals(e, f(v));
}

function foo(t) {
  for (var x in t) {
    for (var i = 0; i < 2; i++) {
      %OptimizeOsr();
    }
  }
  return 5;
}

test(5, foo, {x:20});

function bar(t) {
  var sum = 0;
  for (var x in t) {
    for (var i = 0; i < 2; i++) {
      %OptimizeOsr();
      sum += t[x];
    }
  }
  return sum;
}

test(62, bar, {x:20,y:11});
