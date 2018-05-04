// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --use-osr

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/osr-literals-adapted.js");
	}
}
function mod() {
  function f0() {
    for (var i = 0; i < 3; i = i + 1 | 0) {
      %OptimizeOsr();
    }
    return {blah: i};
  }

  function f1(a) {
    for (var i = 0; i < 3; i = i + 1 | 0) {
      %OptimizeOsr();
    }
    return {blah: i};
  }

  function f2(a,b) {
    for (var i = 0; i < 3; i = i + 1 | 0) {
      %OptimizeOsr();
    }
    return {blah: i};
  }

  function f3(a,b,c) {
    for (var i = 0; i < 3; i = i + 1 | 0) {
      %OptimizeOsr();
    }
    return {blah: i};
  }

  function f4(a,b,c,d) {
    for (var i = 0; i < 3; i = i + 1 | 0) {
      %OptimizeOsr();
    }
    return {blah: i};
  }

  function bar() {
    assertEquals(3, f0().blah);
    assertEquals(3, f1().blah);
    assertEquals(3, f2().blah);
    assertEquals(3, f3().blah);
    assertEquals(3, f4().blah);
  }
  bar();
}


mod();
mod();
mod();
