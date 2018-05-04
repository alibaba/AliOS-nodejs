// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-700678.js");
	}
}
(function testNonConfigurableProperty() {
  function ownKeys(x) { return ["23", "length"]; }
  var target = [];
  var proxy = new Proxy(target, {ownKeys:ownKeys});
  Object.defineProperty(target, "23", {value:true});
  assertEquals(["23", "length"], Object.getOwnPropertyNames(proxy));
})();

(function testPreventedExtension() {
  function ownKeys(x) { return ["42", "length"]; }
  var target = [];
  var proxy = new Proxy(target, {ownKeys:ownKeys});
  target[42] = true;
  Object.preventExtensions(target);
  assertEquals(["42", "length"], Object.getOwnPropertyNames(proxy));
})();
