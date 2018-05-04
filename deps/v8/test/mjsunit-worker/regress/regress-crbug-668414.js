// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-668414.js");
	}
}
(function testSmiArrayConcat() {
  var result = [].concat([-12]);

  assertEquals(1, result.length);
  assertEquals([-12], result);
})();

(function testDoubleArrayConcat() {
  var result = [].concat([-1073741825]);

  assertEquals(1, result.length);
  assertEquals([-1073741825], result);
})();

(function testSmiArrayNonConcatSpreadable() {
  var array = [-10];
  array[Symbol.isConcatSpreadable] = false;
  var result = [].concat(array);

  assertEquals(1, result.length);
  assertEquals(1, result[0].length);
  assertEquals([-10], result[0]);
})();

(function testDoubleArrayNonConcatSpreadable() {
  var array = [-1073741825];
  array[Symbol.isConcatSpreadable] = false;
  var result = [].concat(array);

  assertEquals(1, result.length);
  assertEquals(1, result[0].length);
  assertEquals([-1073741825], result[0]);
})();

Array.prototype[Symbol.isConcatSpreadable] = false;


(function testSmiArray() {
  var result = [].concat([-12]);

  assertEquals(2, result.length);
  assertEquals(0, result[0].length);
  assertEquals(1, result[1].length);
  assertEquals([-12], result[1]);
})();

(function testDoubleArray() {
  var result = [].concat([-1073741825]);

  assertEquals(2, result.length);
  assertEquals(0, result[0].length);
  assertEquals(1, result[1].length);
  assertEquals([-1073741825], result[1]);
})();
