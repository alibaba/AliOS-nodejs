// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/proxies-object-assign.js");
	}
}
var handler = {
  ownKeys: function(t) { return ["a", "b"]; },
  getOwnPropertyDescriptor: function(t, p) {
    return {enumerable: true, configurable: true}
  },
  get: function(t, p) {
    return 1;
  }
};

var proxy = new Proxy({}, handler);

var o = {};

Object.assign(o, proxy);

assertEquals({"a": 1, "b": 1}, o);

(function TestStringSources() {
  var source = "abc";
  var target = {};
  Object.assign(target, source);
  assertEquals({0: "a", 1: "b", 2: "c"}, target);
})();
